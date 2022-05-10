//通过libevent编写的web服务器
#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <fcntl.h>
#include <cstring>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <vector>
#include "Sudoku.h"
#include "pub.h"
#include "wrap.h"
#include <event.h>
#include <event2/listener.h>
#include <dirent.h>

using namespace std;


int copy_header(struct bufferevent* bev, int op, char* msg, char* filetype, long filesize) {
	char buf[4096] = { 0 };
	sprintf(buf, "HTTP/1.1 %d %s\r\n", op, msg);
	sprintf(buf, "%sContent-Type: %s\r\n", buf, filetype);
	if(filesize >= 0) {
		sprintf(buf, "%sContent-Length:%ld\r\n", buf, filesize);
	}
	strcat(buf, "\r\n");
	bufferevent_write(bev, buf, strlen(buf));
	return 0;
}

int copy_file(struct bufferevent* bev, const char* strFile) {
	int fd = open(strFile, O_RDONLY);
	char buf[1024] = { 0 };
	int ret;
	while((ret = read(fd, buf, sizeof(buf))) > 0) {
		bufferevent_write(bev, buf, ret);
	}
	close(fd);
	return 0;
}

int copy_sudoku_result(struct bufferevent* bev, const char* strFile, vector<vector<int> > matrix) {
	int fd = open(strFile, O_RDONLY);
	char buf[1024] = { 0 };
	int ret;
	while(1) {
		ret = Readline(fd, buf, sizeof(buf));
		if(ret < 0) {
			perror("read");
			break;
		}
		if(ret == 0) {
			break;
		}
		bufferevent_write(bev, buf, ret);
		string s1 = buf;
		string s2 = "  var ret = [];";

		if(s1.find(s2) == 0) {
			for(int i = 0;i < 9;i++) {
				for(int j = 0;j < 9;j++) {
					int len = sprintf(buf, "  ret[%d] = %d;\n", 9 * i + j, matrix[i][j]);
					bufferevent_write(bev, buf, len);
				}
			}
		}
	}
	close(fd);
	return 0;
}

bool parse_content(char *content, vector<vector<int> > &sudoku_matrix) {
	string s_content = content;
	if(s_content[s_content.size() - 1] == '?')
		return false;
	if(s_content.find("/?") != 0 && s_content.find("/sudoku.html?") != 0)
		return false;
	sudoku_matrix = vector<vector<int> >(9, vector<int>(9));
	s_content = s_content.substr(s_content.find("?") + 1);
	istringstream iss(s_content);
	string s;
	while(getline(iss, s, '&')) {
		int val;
		if(s[s.size() - 1] == '=')
			val = 0;
		else
			val = s[s.size() - 1] - '0';
		int idx = stoi(s.substr(1, s.find("=") - 1));
		int m = idx / 9;
		int n = idx % 9;
		sudoku_matrix[m][n] = val;
	}
	return true;
}

int http_request(struct bufferevent* bev, char* path) {
	strdecode(path, path);//将中文问题转码成utf-8格式的字符串
	vector<vector<int> > sudoku_matrix;
	if(parse_content(path, sudoku_matrix)) {
		int matrix[9][9];
		bool valid = false;
		bool invalid = false;
		memset(matrix, 0, sizeof(matrix));
		for(int i = 0;i < 9;i++) {
			for(int j = 0;j < 9;j++) {
				matrix[i][j] = sudoku_matrix[i][j];
				if(matrix[i][j] > 0)
					valid = true;
				if(matrix[i][j] < 0 || matrix[i][j] > 9)
					invalid = true;
			}
		}
		struct stat sb;
		stat("sudoku.html", &sb); 
		if(invalid || !valid) {
			copy_header(bev, 200, "OK", get_mime_type(".html"), sb.st_size);
			copy_file(bev, "sudoku.html");
			return 0;
		}
		Sudoku s(matrix);
		s.Solve();
		if(s.solve_result.size() > 0) {
			copy_header(bev, 200, "OK", get_mime_type(".html"), -1);
			copy_sudoku_result(bev, "sudoku_result.html", s.solve_result[0]);
			return 0;
		}
		copy_header(bev, 200, "OK", get_mime_type(".html"), sb.st_size);
		copy_file(bev, "sudoku.html");
		return 0;
	}

	char* strPath = path;
    if(strcmp(strPath, "/") == 0 || strcmp(strPath, "/.") == 0) {
        strPath = "./sudoku.html";
    } else {
        strPath = path + 1;
    }
	string temp = strPath;
	if(temp[temp.size() - 1] == '?') {
		strcpy(strPath, temp.substr(0, temp.size() - 1).c_str());
	}
    struct stat sb;

    if(stat(strPath, &sb) < 0) {
        //不存在 ，给404页面
        copy_header(bev, 404, "NOT FOUND", get_mime_type("error.html"), -1);
        copy_file(bev, "error.html");
        return -1;
    }
    if(S_ISREG(sb.st_mode)) {
        //处理文件
        //写头
        copy_header(bev, 200, "OK", get_mime_type(strPath), sb.st_size);
        //写文件内容
        copy_file(bev, strPath);
    }

    return 0;
}

void read_cb(struct bufferevent* bev, void* ctx) {
    char buf[1024] = { 0 };
    char method[10], path[1024], protocol[10];
    int ret = bufferevent_read(bev, buf, sizeof(buf));
    if(ret > 0) {

        sscanf(buf, "%[^ ] %[^ ] %[^ \r\n]", method, path, protocol);
        if(strcasecmp(method, "get") == 0) {
            //处理客户端的请求
            char bufline[1024];
            write(STDOUT_FILENO, buf, ret);
            //确保数据读完
            while((ret = bufferevent_read(bev, bufline, sizeof(bufline))) > 0) {
				write(STDOUT_FILENO, bufline, ret);
            }
            http_request(bev, path);//处理请求

        }
    }
}

void bevent_cb(struct bufferevent* bev, short what, void* ctx) {
    if(what & BEV_EVENT_EOF) {//客户端关闭
        printf("client closed\n");
        bufferevent_free(bev);
    } else if(what & BEV_EVENT_ERROR) {
        printf("err to client closed\n");
        bufferevent_free(bev);
    } else if(what & BEV_EVENT_CONNECTED) {//连接成功
        printf("client connect ok\n");
    }
}

void listen_cb(struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* addr, int socklen, void* arg) {
	char ip[16];
	struct sockaddr_in cliaddr = *(struct sockaddr_in*)addr;
	cout << "new client ip: " << inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, ip, 16) << endl;
	cout << "new client port: " << ntohs(cliaddr.sin_port) << endl;
    //定义与客户端通信的bufferevent
    struct event_base* base = (struct event_base*) arg;
    struct bufferevent* bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, read_cb, NULL, bevent_cb, base);//设置回调
    bufferevent_enable(bev, EV_READ | EV_WRITE);//启用读和写
}

int main(int argc, char* argv[]) {
    char workdir[256] = { 0 };
    strcpy(workdir, getenv("PWD"));
	strcat(workdir, "/web-http");    
    cout << workdir << endl;
    chdir(workdir);
    struct event_base* base = event_base_new();//创建根节点
    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(9999);
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
    struct evconnlistener* listener = evconnlistener_new_bind(base,
        listen_cb, base, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
        (struct sockaddr*) &serv, sizeof(serv));//连接监听器

    event_base_dispatch(base);//循环
    event_base_free(base); //释放根节点
    evconnlistener_free(listener);//释放链接监听器
    return 0;
}
