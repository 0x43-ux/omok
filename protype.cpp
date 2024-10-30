#include <iostream>
#include <vector>
#include <limits.h>
#include <unistd.h> // sleep 함수를 사용하기 위해 필요

#define SIZE 15
#define MAX_DEPTH 3 // 탐색 깊이를 3으로 제한

using namespace std;

enum Player { EMPTY = 0, HUMAN = 1, COMPUTER = 2 };

// 게임 보드 초기화
vector<vector<int> > board(SIZE, vector<int>(SIZE, EMPTY));

void printBoard() {
    // 위쪽 숫자 출력
    cout << "  ";
    for (int i = 0; i < SIZE; ++i) {
        if (i < 10) cout << " " << i << "";
        else cout << i << "";
    }
    cout << endl;

    for (int i = 0; i < SIZE; ++i) {
        // 왼쪽 숫자 출력
        if (i < 10) cout << " " << i << " ";
        else cout << i << " ";

        for (int j = 0; j < SIZE; ++j) {
            if (board[i][j] == EMPTY) cout << ".";
            else if (board[i][j] == HUMAN) cout << "O";
            else cout << "X";
            cout << " ";
        }
        cout << endl;
    }
}

bool checkWin(int player) {
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            if (board[i][j] == player) {
                // 가로 체크
                if (j <= SIZE - 5 && board[i][j + 1] == player && board[i][j + 2] == player &&
                    board[i][j + 3] == player && board[i][j + 4] == player)
                    return true;

                // 세로 체크
                if (i <= SIZE - 5 && board[i + 1][j] == player && board[i + 2][j] == player &&
                    board[i + 3][j] == player && board[i + 4][j] == player)
                    return true;

                // 대각선 체크
                if (i <= SIZE - 5 && j <= SIZE - 5 && board[i + 1][j + 1] == player &&
                    board[i + 2][j + 2] == player && board[i + 3][j + 3] == player &&
                    board[i + 4][j + 4] == player)
                    return true;

                // 역대각선 체크
                if (i >= 4 && j <= SIZE - 5 && board[i - 1][j + 1] == player &&
                    board[i - 2][j + 2] == player && board[i - 3][j + 3] == player &&
                    board[i - 4][j + 4] == player)
                    return true;
            }
        }
    }
    return false;
}

bool isBoardFull() {
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            if (board[i][j] == EMPTY) return false;
        }
    }
    return true;
}

int evaluateBoard() {
    if (checkWin(COMPUTER)) return 1000;
    if (checkWin(HUMAN)) return -1000;
    return 0;
}

int minimax(int depth, bool isMaximizing, int alpha, int beta) {
    int score = evaluateBoard();

    if (score == 1000 || score == -1000) return score;
    if (isBoardFull()) return 0;
    if (depth >= MAX_DEPTH) return score; // 탐색 깊이 제한

    if (isMaximizing) {
        int best = INT_MIN;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (board[i][j] == EMPTY) {
                    board[i][j] = COMPUTER;
                    best = max(best, minimax(depth + 1, false, alpha, beta));
                    board[i][j] = EMPTY;
                    alpha = max(alpha, best);
                    if (beta <= alpha)
                        break;
                }
            }
        }
        return best;
    } else {
        int best = INT_MAX;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (board[i][j] == EMPTY) {
                    board[i][j] = HUMAN;
                    best = min(best, minimax(depth + 1, true, alpha, beta));
                    board[i][j] = EMPTY;
                    beta = min(beta, best);
                    if (beta <= alpha)
                        break;
                }
            }
        }
        return best;
    }
}

pair<int, int> findBestMove() {
    int bestVal = INT_MIN;
    pair<int, int> bestMove = make_pair(-1, -1);

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (board[i][j] == EMPTY) {
                board[i][j] = COMPUTER;

                // 컴퓨터가 계산 중임을 표시
                cout << ".";
                cout.flush();

                int moveVal = minimax(0, false, INT_MIN, INT_MAX);
                board[i][j] = EMPTY;
                if (moveVal > bestVal) {
                    bestMove = make_pair(i, j);
                    bestVal = moveVal;
                }
            }
        }
    }
    cout << endl;  // 점 표시가 끝난 후 줄 바꿈
    return bestMove;
}

int main() {
    cout << "오목 게임 시작!" << endl;
    printBoard();

    while (true) {
        int x, y;
        cout << "당신의 차례입니다. 위치를 입력하세요 (x y): ";
        cin >> x >> y;

        if (x < 0 || x >= SIZE || y < 0 || y >= SIZE || board[x][y] != EMPTY) {
            cout << "잘못된 위치입니다. 다시 입력하세요." << endl;
            continue;
        }

        board[x][y] = HUMAN;

        if (checkWin(HUMAN)) {
            printBoard();
            cout << "당신이 이겼습니다!" << endl;
            break;
        }

        pair<int, int> bestMove = findBestMove();
        board[bestMove.first][bestMove.second] = COMPUTER;

        if (checkWin(COMPUTER)) {
            printBoard();
            cout << "컴퓨터가 이겼습니다!" << endl;
            break;
        }

        printBoard();

        if (isBoardFull()) {
            cout << "무승부입니다!" << endl;
            break;
        }
    }

    return 0;
}
