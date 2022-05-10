#include "Sudoku.h"
#include <iostream>
using namespace std;

Sudoku::Sudoku(int matrix[N][N]) {
	found = false;
	recursion_num = 0;
	InitSoduku(matrix);
}

void Sudoku::InitSoduku(int matrix[N][N]) {
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			sudoku_matrix[i][j] = matrix[i][j];
			if (matrix[i][j] != 0)
				confirm_matrix[i][j] = true;
			else
				confirm_matrix[i][j] = false;
		}
	}
}

bool Sudoku::CheckMatrix() {
	for(int i = 0; i < N; i++) {
		for(int j = 0; j < N; j++) {
			if(sudoku_matrix[i][j] == 0)
				continue;
			for (int k = 0; k < N; k++) {
				if (k != j && sudoku_matrix[i][k] == sudoku_matrix[i][j])
					return false;
				if (k != i && sudoku_matrix[k][j] == sudoku_matrix[i][j])
					return false;
			}

			for (int m = i / M * M; m < i / M * M + M; m++) {
				for (int n = j / M * M; n < j / M * M + M; n++) {
					if ((m != i || n != j) && sudoku_matrix[m][n] == sudoku_matrix[i][j])
						return false;
				}
			}
		}
	}
	return true;
}

void Sudoku::UpdatePotentialMatrix() {
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			for (int k = 0; k < N; k++) {
				potential_matrix[i][j][k] = true;
			}
		}
	}

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			if (confirm_matrix[i][j]) {
				for (int k = 0; k < N; k++) {
					if (k != sudoku_matrix[i][j] - 1)
						potential_matrix[i][j][k] = false;
				}
				continue;
			}
			for (int k = 0; k < N; k++) {
				if (sudoku_matrix[i][k] != 0)
					potential_matrix[i][j][sudoku_matrix[i][k] - 1] = false;
				if (sudoku_matrix[k][j] != 0)
					potential_matrix[i][j][sudoku_matrix[k][j] - 1] = false;
			}

			for (int m = i / M * M; m < i / M * M + M; m++) {
				for (int n = j / M * M; n < j / M * M + M; n++) {
					if (sudoku_matrix[m][n] != 0)
						potential_matrix[i][j][sudoku_matrix[m][n] - 1] = false;
				}
			}
		}
	}

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			int temp_num = 0;
			for (int k = 0; k < N; k++) {
				if (potential_matrix[i][j][k])
					temp_num++;
			}
			potential_num_matrix[i][j] = temp_num;
		}
	}
}

void Sudoku::GetNextPos(int &x, int &y) {
	int min_num = N;
	int index_x, index_y;
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			if (confirm_matrix[i][j])
				continue;
			if (min_num > potential_num_matrix[i][j]) {
				min_num = potential_num_matrix[i][j];
				index_x = i;
				index_y = j;
			}
		}
	}
	x = index_x;
	y = index_y;
}

void Sudoku::AddSudokuMatrix(int x, int y, int value) {
	confirm_matrix[x][y] = true;
	sudoku_matrix[x][y] = value;
	for (int k = 0; k < N; k++) {
		if (k != value - 1)
			potential_matrix[x][y][k] = false;
	}
	potential_num_matrix[x][y] = 1;

	for (int k = 0; k < N; k++) {
		if (!confirm_matrix[x][k] && k != y) {
			if (potential_matrix[x][k][value - 1])
				potential_num_matrix[x][k]--;
			potential_matrix[x][k][value - 1] = false;
		}
		if (!confirm_matrix[k][y] && x != k) {
			if (potential_matrix[k][y][value - 1])
				potential_num_matrix[k][y]--;
			potential_matrix[k][y][value - 1] = false;
		}
	}

	for (int m = x / M * M; m < x / M * M + M; m++) {
		for (int n = y / M * M; n < y / M * M + M; n++) {
			if (!confirm_matrix[m][n] && !(m == x && n == y)) {
				if (potential_matrix[m][n][value - 1])
					potential_num_matrix[m][n]--;
				potential_matrix[m][n][value - 1] = false;
			}
		}
	}
}

void Sudoku::DeleteSudokuMatrix(int x, int y) {
	int delete_value = sudoku_matrix[x][y];
	confirm_matrix[x][y] = false;
	sudoku_matrix[x][y] = 0;
	UpdatePotentialMatrix();
}

void Sudoku::SudokuDFS(int x, int y, int value, int step) {
	if(found)
		return;
	recursion_num++;
	step++;
	AddSudokuMatrix(x, y, value);
	if (step == N * N) {
		Print();
		vector<vector<int> > temp(N, vector<int>(N));
		for(int i = 0;i < N;i++) {
			for(int j = 0;j < N;j++) {
				temp[i][j] = sudoku_matrix[i][j];
			}
		}
		found = true;
		solve_result.push_back(temp);
	}
	else {
		int next_x, next_y;
		GetNextPos(next_x, next_y);
		for (int k = 0; k < N; k++) {
			if (potential_matrix[next_x][next_y][k])
				SudokuDFS(next_x, next_y, k + 1, step);
		}
	}
	step--;
	DeleteSudokuMatrix(x, y);
	
}

void Sudoku::Solve() {
	if(!CheckMatrix())
		return;
	UpdatePotentialMatrix();
	int start_x, start_y, start_value, start_step = 0;
	GetNextPos(start_x, start_y);
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			if (confirm_matrix[i][j])
				start_step++;
		}
	}
	for (int k = 0; k < N; k++) {
		if (potential_matrix[start_x][start_y][k]) {
			start_value = k + 1;
			break;
		}
	}
	SudokuDFS(start_x, start_y, start_value, start_step);
}

void Sudoku::Print() {
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			if (sudoku_matrix[i][j] == 0) {
				cout << "  ";
			}
			else {
				cout << sudoku_matrix[i][j] << " ";
			}
		}
		cout << endl;
	}
	cout << endl;
}
