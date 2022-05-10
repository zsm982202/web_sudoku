#ifndef _SUDOKU_H
#define _SUDOKU_H
#include <vector>
class Sudoku {
public:
	const static int N = 9;
	const static int M = 3;
	int recursion_num;
	std::vector<std::vector<std::vector<int> > > solve_result;
	Sudoku(int matrix[N][N]);
	void Solve();
	void Print();
private:
	bool found;
	int sudoku_matrix[N][N];
	bool potential_matrix[N][N][N];
	int potential_num_matrix[N][N];
	bool confirm_matrix[N][N];
	void InitSoduku(int matrix[N][N]);
	bool CheckMatrix();
	void UpdatePotentialMatrix();
	void AddSudokuMatrix(int x, int y, int value);
	void DeleteSudokuMatrix(int x, int y);
	void GetNextPos(int &x, int &y);
	void SudokuDFS(int x, int y, int value, int step);
};
#endif // !_SUDOKU_H
