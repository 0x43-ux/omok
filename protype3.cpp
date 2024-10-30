#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <limits>
#include <iomanip>
#include <thread>
#include <mutex>

const int BOARD_SIZE = 15;

enum class Player { NONE, PLAYER1, PLAYER2 };

class GameState {
public:
    std::vector<std::vector<Player>> board;
    Player currentPlayer;

    GameState() : board(BOARD_SIZE, std::vector<Player>(BOARD_SIZE, Player::NONE)), currentPlayer(Player::PLAYER1) {}

    bool isTerminal() const {
        return checkWin() != Player::NONE || getPossibleMoves().empty();
    }

    std::vector<GameState> getPossibleMoves() const {
        std::vector<GameState> moves;
        for (int i = 0; i < BOARD_SIZE; ++i) {
            for (int j = 0; j < BOARD_SIZE; ++j) {
                if (board[i][j] == Player::NONE) {
                    GameState newState = *this;
                    newState.board[i][j] = currentPlayer;
                    newState.currentPlayer = (currentPlayer == Player::PLAYER1) ? Player::PLAYER2 : Player::PLAYER1;
                    moves.push_back(newState);
                }
            }
        }
        return moves;
    }

    Player getWinner() const {
        return checkWin();
    }

private:
    Player checkWin() const {
        for (int i = 0; i < BOARD_SIZE; ++i) {
            for (int j = 0; j < BOARD_SIZE; ++j) {
                if (board[i][j] != Player::NONE) {
                    if (checkDirection(i, j, 1, 0) || checkDirection(i, j, 0, 1) ||
                        checkDirection(i, j, 1, 1) || checkDirection(i, j, 1, -1)) {
                        return board[i][j];
                    }
                }
            }
        }
        return Player::NONE;
    }

    bool checkDirection(int x, int y, int dx, int dy) const {
        Player start = board[x][y];
        for (int step = 1; step < 5; ++step) {
            int nx = x + step * dx;
            int ny = y + step * dy;
            if (nx < 0 || ny < 0 || nx >= BOARD_SIZE || ny >= BOARD_SIZE || board[nx][ny] != start) {
                return false;
            }
        }
        return true;
    }
};

class Node {
public:
    GameState state;
    Node* parent;
    std::vector<Node*> children;
    int wins;
    int visits;

    Node(const GameState& state, Node* parent = nullptr)
        : state(state), parent(parent), wins(0), visits(0) {}

    bool isFullyExpanded() const {
        return children.size() == state.getPossibleMoves().size();
    }

    Node* bestChild() const {
        double bestValue = -std::numeric_limits<double>::infinity();
        Node* bestNode = nullptr;
        for (Node* child : children) {
            double uctValue = double(child->wins) / (child->visits + 1e-6) +
                              std::sqrt(2.0 * std::log(visits + 1) / (child->visits + 1e-6));
            if (uctValue > bestValue) {
                bestValue = uctValue;
                bestNode = child;
            }
        }
        return bestNode;
    }
};

class MCTS {
public:
    GameState run(const GameState& initialState, int iterations, int numThreads) {
        Node* root = new Node(initialState);
        std::vector<std::thread> threads;
        std::mutex mtx;
        int completedIterations = 0;

        auto worker = [&](int threadId) {
            for (int i = 0; i < iterations / numThreads; ++i) {
                Node* node = treePolicy(root);
                Player result = defaultPolicy(node->state);
                backpropagate(node, result);

                {
                    std::lock_guard<std::mutex> lock(mtx);
                    completedIterations++;
                    if (completedIterations % (iterations / 100) == 0) {
                        std::cout << "Progress: " << std::setw(3) << (completedIterations * 100) / iterations << "%\r";
                        std::cout.flush();
                    }
                }
            }
        };

        for (int i = 0; i < numThreads; ++i) {
            threads.emplace_back(worker, i);
        }

        for (auto& t : threads) {
            t.join();
        }

        Node* bestChild = root->bestChild();
        GameState bestState = bestChild->state;

        deleteTree(root);
        return bestState;
    }

private:
    Node* treePolicy(Node* node) {
        while (!node->state.isTerminal()) {
            if (!node->isFullyExpanded()) {
                return expand(node);
            } else {
                node = node->bestChild();
            }
        }
        return node;
    }

    Node* expand(Node* node) {
        std::vector<GameState> possibleMoves = node->state.getPossibleMoves();
        for (const GameState& move : possibleMoves) {
            bool alreadyExpanded = false;
            for (Node* child : node->children) {
                if (child->state.board == move.board) {
                    alreadyExpanded = true;
                    break;
                }
            }
            if (!alreadyExpanded) {
                Node* childNode = new Node(move, node);
                node->children.push_back(childNode);
                return childNode;
            }
        }
        return nullptr;
    }

    Player defaultPolicy(GameState state) {
        while (!state.isTerminal()) {
            std::vector<GameState> possibleMoves = state.getPossibleMoves();
            state = possibleMoves[rand() % possibleMoves.size()];
        }
        return state.getWinner();
    }

    void backpropagate(Node* node, Player result) {
        while (node != nullptr) {
            node->visits++;
            if (node->state.currentPlayer != result) {
                node->wins++;
            }
            node = node->parent;
        }
    }

    void deleteTree(Node* node) {
        for (Node* child : node->children) {
            deleteTree(child);
        }
        delete node;
    }
};

class Gomoku {
private:
    GameState gameState;

public:
    void printBoard() const {
        std::cout << "   ";
        for (int i = 0; i < BOARD_SIZE; ++i) {
            std::cout << std::setw(2) << i << " ";
        }
        std::cout << std::endl;

        for (int i = 0; i < BOARD_SIZE; ++i) {
            std::cout << std::setw(2) << i << " ";
            for (int j = 0; j < BOARD_SIZE; ++j) {
                char c = gameState.board[i][j] == Player::PLAYER1 ? 'X' : (gameState.board[i][j] == Player::PLAYER2 ? 'O' : '.');
                std::cout << " " << c << " ";
            }
            std::cout << " " << std::setw(2) << i << std::endl;
        }

        std::cout << "   ";
        for (int i = 0; i < BOARD_SIZE; ++i) {
            std::cout << std::setw(2) << i << " ";
        }
        std::cout << std::endl;
    }

    void playerMove(int x, int y) {
        if (gameState.board[x][y] == Player::NONE) {
            gameState.board[x][y] = Player::PLAYER1;
            gameState.currentPlayer = Player::PLAYER2;
        } else {
            std::cout << "Invalid move. Try again.\n";
        }
    }

    void aiMove(int numThreads) {
        MCTS mcts;
        gameState = mcts.run(gameState, 1000, numThreads);  // MCTS로 1000번 시뮬레이션
        gameState.currentPlayer = Player::PLAYER1;
    }

    Player checkWinner() const {
        return gameState.getWinner();
    }

    bool isGameOver() const {
        return gameState.isTerminal();
    }
};

int main() {
    srand(static_cast<unsigned int>(time(0)));
    Gomoku game;

    const int numThreads = std::thread::hardware_concurrency();

    while (!game.isGameOver()) {
        int x, y;
        game.printBoard();

        std::cout << "Enter your move (x y): ";
        std::cin >> x >> y;
        game.playerMove(x, y);

        if (game.checkWinner() != Player::NONE) {
            game.printBoard();
            std::cout << "Player 1 wins!\n";
            break;
        }

        std::cout << "AI is thinking...\n";
        game.aiMove(numThreads);

        if (game.checkWinner() != Player::NONE) {
            game.printBoard();
            std::cout << "AI wins!\n";
            break;
        }
    }

    if (game.isGameOver() && game.checkWinner() == Player::NONE) {
        std::cout << "It's a draw!\n";
    }

    return 0;
}



//cd "/Users/wonjoun/developement/cpp/오목/" && g++ -std=c++11 protype2.cpp -o protype2 && "/Users/wonjoun/developement/cpp/오목/"protype2
