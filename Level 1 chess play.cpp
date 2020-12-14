#include <iostream>
#include <vector>
#include <unordered_map>
#include <ctime>
#include <cstdio>
#include <algorithm>

using namespace std;

typedef vector<int> Board;

#define CHECK_SAME_COLOR(c1, c2)  (((c1)  < 0 && (c2) < 0) || ((c1)  > 0 && (c2) > 0 && (c1) != BEDROCK && (c2) != BEDROCK))
#define MOVE_SCORE(move, board) (move.value - piece_values[board[move.start]] - piece_values[board[move.end]])


const int BOARD_SIZE = 12;
int MAX_DEPTH = 6;

const int EMPTY = 0;
const int KING = 1;
const int QUEEN = 2;
const int ROOK = 3;
const int BISHOP = 4;
const int KNIGHT = 5;
const int PAWN = 6;
const int BEDROCK = 7;

const int WHITE = 1;
const int BLACK = -1;

struct One_move {
	int start;
	int end;
	int value;
};


vector<int> non_bedrock_cells;
vector<int> white_promote{ QUEEN, ROOK, BISHOP, KNIGHT };
vector<int> black_promote{ -QUEEN, -ROOK, -BISHOP, -KNIGHT };
vector<int> position_score{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
	0, 0, 1, 2, 2, 2, 2, 2, 2, 1, 0, 0,
	0, 0, 1, 2, 3, 3, 3, 3, 2, 1, 0, 0,
	0, 0, 1, 2, 3, 5, 5, 3, 2, 1, 0, 0,
	0, 0, 1, 2, 3, 5, 5, 3, 2, 1, 0, 0,
	0, 0, 1, 2, 3, 3, 3, 3, 2, 1, 0, 0,
	0, 0, 1, 2, 2, 2, 2, 2, 2, 1, 0, 0,
	0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
vector<int> white_pawn_pos_score{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 5, 5, 5, 5, 5, 5, 5, 5, 0, 0,
	0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0,
	0, 0, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0,
	0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0,
	0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

vector<int> black_pawn_pos_score{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0,
	0, 0, -2, -2, -2, -2, -2, -2, -2, -2, 0, 0,
	0, 0, -3, -3, -3, -3, -3, -3, -3, -3, 0, 0,
	0, 0, -4, -4, -4, -4, -4, -4, -4, -4, 0, 0,
	0, 0, -5, -5, -5, -5, -5, -5, -5, -5, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

bool operator ==(const One_move& om1, const One_move& om2) {
	return (om1.start == om2.start && om1.end == om2.end && om1.value == om2.value);
}


unordered_map<char, int> promote{
{'Q', QUEEN},
{'R', ROOK},
{'B', BISHOP},
{'N', KNIGHT}
};

unordered_map<int, vector<int>> tmp_move{
	{KING, {-1, 1, BOARD_SIZE, -BOARD_SIZE, BOARD_SIZE - 1, BOARD_SIZE + 1, -BOARD_SIZE - 1, -BOARD_SIZE + 1}},
	{ROOK, {-1, 1, BOARD_SIZE, -BOARD_SIZE}},
	{BISHOP, {-BOARD_SIZE - 1, -BOARD_SIZE + 1, BOARD_SIZE + 1, BOARD_SIZE - 1}},
	{KNIGHT, {-2 * BOARD_SIZE + 1, -2 * BOARD_SIZE - 1, -BOARD_SIZE - 2, -BOARD_SIZE + 2, 2 * BOARD_SIZE + 1, 2 * BOARD_SIZE - 1, BOARD_SIZE - 2, BOARD_SIZE + 2}}
};
unordered_map<int, vector<int>> piece_move{
	{KING, tmp_move[KING]},
	{QUEEN, tmp_move[KING]},
	{ROOK, tmp_move[ROOK]},
	{BISHOP, tmp_move[BISHOP]},
	{KNIGHT, tmp_move[KNIGHT]},
	{PAWN, {-BOARD_SIZE }},
	{-KING, tmp_move[KING]},
	{-QUEEN, tmp_move[KING]},
	{-ROOK, tmp_move[ROOK]},
	{-BISHOP, tmp_move[BISHOP]},
	{-KNIGHT, tmp_move[KNIGHT]},
	{-PAWN, { BOARD_SIZE }},
	{EMPTY, {}},
	{BEDROCK, {}}
};

unordered_map<int, int> piece_values{
	{EMPTY, 0},
	{BEDROCK, 0},
	{KING, 1000},
	{QUEEN, 9},
	{KNIGHT, 3},
	{BISHOP, 3},
	{PAWN, 1},
	{ROOK, 5},
	{-KING, -1000},
	{-QUEEN, -9},
	{-KNIGHT, -3},
	{-BISHOP, -3},
	{-PAWN, -1},
	{-ROOK, -5},
};
unordered_map<int, string> piece_to_print{
	{KING, "wK"},
	{QUEEN, "wQ"},
	{ROOK, "wR"},
	{BISHOP, "wB"},
	{KNIGHT, "wN"},
	{PAWN, "wP"},
	{-KING, "bK"},
	{-QUEEN, "bQ"},
	{-ROOK, "bR"},
	{-BISHOP, "bB"},
	{-KNIGHT, "bN"},
	{-PAWN, "bP"},
	{EMPTY, "  "}
};
unordered_map<int, bool> repeated_piece{
	{KING, false},
	{QUEEN, true},
	{ROOK, true},
	{BISHOP, true},
	{KNIGHT, false},
	{-KING, false},
	{-QUEEN, true},
	{-ROOK, true},
	{-BISHOP, true},
	{-KNIGHT, false}
};

unordered_map<int, int> position_piece_score{
	{ROOK, 1},
	{BISHOP, 2},
	{KNIGHT, 2},
	{PAWN, 4},
	{QUEEN, 0},
	{KING, -1},
	{-ROOK, -1},
	{-BISHOP, -2},
	{-KNIGHT, -2},
	{-PAWN, -4},
	{-QUEEN, 0},
	{-KING, 1},
	{EMPTY, 0}
};


Board create_initial_board() {
	return {
		BEDROCK, BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,
		BEDROCK, BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,
		BEDROCK, BEDROCK,-ROOK, -KNIGHT, -BISHOP, -QUEEN, -KING, -BISHOP, -KNIGHT, -ROOK,BEDROCK,BEDROCK,
		BEDROCK, BEDROCK,-PAWN, -PAWN,-PAWN,-PAWN,-PAWN,-PAWN,-PAWN,-PAWN,BEDROCK,BEDROCK,
		BEDROCK, BEDROCK,EMPTY, EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,BEDROCK,BEDROCK,
		BEDROCK, BEDROCK,EMPTY, EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,BEDROCK,BEDROCK,
		BEDROCK, BEDROCK,EMPTY, EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,BEDROCK,BEDROCK,
		BEDROCK, BEDROCK,EMPTY, EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,BEDROCK,BEDROCK,
		BEDROCK, BEDROCK,PAWN, PAWN,PAWN,PAWN,PAWN,PAWN,PAWN,PAWN,BEDROCK,BEDROCK,
		BEDROCK, BEDROCK,ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK,BEDROCK,BEDROCK,
		BEDROCK, BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,
		BEDROCK, BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK
	};
}


Board create_initial_board_test() {
	return {
		BEDROCK, BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,
		BEDROCK, BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,
		BEDROCK, BEDROCK,EMPTY, EMPTY,EMPTY,EMPTY,EMPTY,-QUEEN,EMPTY,EMPTY,BEDROCK,BEDROCK,
		BEDROCK, BEDROCK,EMPTY, EMPTY,-KING,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,BEDROCK,BEDROCK,
		BEDROCK, BEDROCK,EMPTY, EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,BEDROCK,BEDROCK,
		BEDROCK, BEDROCK,EMPTY, EMPTY,-PAWN,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,BEDROCK,BEDROCK,
		BEDROCK, BEDROCK,EMPTY, EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,BEDROCK,BEDROCK,
		BEDROCK, BEDROCK,EMPTY, EMPTY,QUEEN,EMPTY,PAWN,EMPTY,EMPTY,EMPTY,BEDROCK,BEDROCK,
		BEDROCK, BEDROCK,EMPTY, EMPTY,EMPTY,EMPTY,KING,EMPTY,EMPTY,EMPTY,BEDROCK,BEDROCK,
		BEDROCK, BEDROCK,EMPTY, EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,BEDROCK,BEDROCK,
		BEDROCK, BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,
		BEDROCK, BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,BEDROCK,
	};
}



vector<int> move_all_but_pawn(int pos, const Board& board) {
	vector<int> output;
	int piece = board[pos];
	for (int move : piece_move[piece]) {
		int new_pos = pos;
		do {
			new_pos += move;
			if (board[new_pos] == BEDROCK) {
				break;
			}
			if (board[new_pos] == EMPTY) {
				output.push_back(new_pos);
			}
			else {
				if (!(CHECK_SAME_COLOR(board[pos], board[new_pos]))) {
					output.push_back(new_pos);
				}
				break;
			}
		} while (repeated_piece[board[pos]]);

	}
	return output;
}

vector<int> move_pawn(int pos, const Board& board) {
	vector<int> output;
	int new_pos = pos;
	int direction = piece_move[board[pos]][0];
	new_pos += direction;
	if (board[new_pos] == EMPTY) {
		output.push_back(new_pos);
		new_pos += direction;
		int row = pos / BOARD_SIZE;
		bool double_move = (board[pos] < 0) ? row == 3 : row == 8;

		if (board[new_pos] == EMPTY && double_move) {
			output.push_back(new_pos);

		}
	}
	new_pos = pos + direction;
	for (int diag : {-1, 2}) {
		new_pos += diag;
		if (board[new_pos] == EMPTY || board[new_pos] == BEDROCK) {
			continue;
		}
		if (!(CHECK_SAME_COLOR(board[new_pos], board[pos]))) {
			output.push_back(new_pos);
		}
	}
	return output;

}


vector<One_move> get_all_moves(const Board& board, int color, vector<One_move>& output) {
	output.clear();
	for (int pos : non_bedrock_cells) {
		int value = board[pos];
		if (value == BEDROCK || value == EMPTY) {
			continue;
		}
		if (!(CHECK_SAME_COLOR(value, color))) {
			continue;
		}
		vector<int> tmp = (abs(value) == PAWN) ? move_pawn(pos, board) : move_all_but_pawn(pos, board);
		for (int new_pos : tmp) {

			if (abs(value) == PAWN) {
				int row = new_pos / BOARD_SIZE;
				if (row == 2 || row == 9) {
					vector<int> t = (color < 0) ? black_promote : white_promote;
					for (int promoting_value : t) {
						output.push_back({ pos, new_pos, promoting_value });
					}
					continue;
				}
			}
			output.push_back({ pos, new_pos, board[pos] });
		}
	}

	sort(output.begin(), output.end(),
		[&board, &color](const One_move& m1, const One_move& m2) {
			int move_one_value = MOVE_SCORE(m1, board);
			int move_two_value = MOVE_SCORE(m2, board);
			return (color == WHITE) ? (move_one_value > move_two_value) : (move_one_value < move_two_value);
		});
	return output;
}

int get_score(const Board& board) {
	int piece_score = 0;
	for (int i : non_bedrock_cells) {
		piece_score += piece_values[board[i]];
	}
	return piece_score;
}

string location_to_notation(int pos) {
	char row = 8 - (pos / BOARD_SIZE - 2) + '0';
	char col = pos % BOARD_SIZE - 2 + 'a';
	string output;

	output += col;
	output += row;
	return output;
}


int to_loc(char file, char rank) {
	int row = (7 - (rank - '1')) + 2;
	int col = (file - 'a') + 2;
	return row * BOARD_SIZE + col;
}


One_move notation_to_location(string s, const Board& board, int color) {

	return {
		to_loc(s[0], s[1]),
		to_loc(s[2], s[3]),
		(s.size() > 4) ? color * promote[s[5]] : board[to_loc(s[0], s[1])]
	};


}


One_move get_users_notation(const Board& board, int color) {
	vector<One_move> tmp;
	get_all_moves(board, color, tmp);
	cout << "\n";
	while (true) {
		string s;
		cout << "Please Move!\n";
		cin >> s;

		One_move humans_move = notation_to_location(s, board, color);
		if (find(tmp.begin(), tmp.end(), humans_move) != tmp.end()) {
			return humans_move;
		}
		cout << "INVALID MOVE \n";
	}
}

vector<int> find_non_bedrock() {
	vector<int> output;
	Board tmp = create_initial_board();
	for (int i = 0; i < tmp.size(); i++) {
		if (tmp[i] != BEDROCK) {
			output.push_back(i);
		}
	}
	return output;
}

void print(const Board& board) {
	cout << "\n\n";
	int count = 8;
	for (int row = 2; row <= 9; row++) {
		cout << "  +--+--+--+--+--+--+--+--+\n";
		cout << count-- << " ";
		for (int col = 2; col <= 9; col++) {
			cout << "|" << piece_to_print[board[row * BOARD_SIZE + col]];
		}
		cout << "|\n";
	}

	cout << "  +--+--+--+--+--+--+--+--+\n   ";


	for (char ch = 'a'; ch < 'h' + 1; ch++) {
		cout << ch << "  ";
	}
	cout << "\n";
	cout << "score = " << get_score(board) << "\n\n";

}

One_move find_best_move_for_white(Board& board) {
	vector<One_move> moves;
	One_move best_move;
	int best_score = std::numeric_limits<int>::min();
	//get all possible move for WHITE
	get_all_moves(board, WHITE, moves);

	for (const One_move& move : moves) {
		int start_value = board[move.start];
		int end_value = board[move.end];
		//make the move
		board[move.start] = EMPTY;
		board[move.end] = move.value;

		int score = get_score(board);
		board[move.start] = start_value;
		board[move.end] = end_value;

		// update the best score
		if (score > best_score || (score == best_score && rand() % 2 == 0)) {
			best_move = move;
			best_score = score;
		}

	}
	return best_move;
}


One_move find_best_move_for_black(Board& board) {
	vector<One_move> moves;
	One_move best_move;
	int best_score = std::numeric_limits<int>::max();
	//get all possible move for WHITE
	get_all_moves(board, BLACK, moves);

	for (const One_move& move : moves) {
		int start_value = board[move.start];
		int end_value = board[move.end];
		//make the move
		board[move.start] = EMPTY;
		board[move.end] = move.value;

		int score = get_score(board);
		board[move.start] = start_value;
		board[move.end] = end_value;

		// update the best score
		if (score < best_score || (score == best_score && rand() % 2 == 0)) {
			best_move = move;
			best_score = score;
		}
	}
	return best_move;
}

void play_game_simple_for_white() {
	Board tmp = create_initial_board_test();
	print(tmp);
	cout << "\nscore = " << get_score(tmp) << "\n";


	while (true) {
		//One_move best_move = find_best_move(WHITE, tmp);
		One_move best_move = find_best_move_for_white(tmp);
		tmp[best_move.start] = EMPTY;
		tmp[best_move.end] = best_move.value;
		print(tmp);
		cout << "\nscore = " << get_score(tmp) << "\n";
		One_move human_move = get_users_notation(tmp, BLACK);
		tmp[human_move.start] = EMPTY;
		tmp[human_move.end] = human_move.value;
		cout << "Score = " << get_score(tmp) << "\n";
		print(tmp);
		cout << "\n";
		cout << "\nscore = " << get_score(tmp) << "\n";

	}
}

void play_game_simple_for_black() {
	Board tmp = create_initial_board_test();
	while (true) {
		One_move best_move = find_best_move_for_black(tmp);
		tmp[best_move.start] = EMPTY;
		tmp[best_move.end] = best_move.value;
		cout << "Score = " << get_score(tmp) << "\n";
		print(tmp);
		cout << "\n";
		One_move human_move = get_users_notation(tmp, WHITE);
		tmp[human_move.start] = EMPTY;
		tmp[human_move.end] = human_move.value;
		cout << "Score = " << get_score(tmp) << "\n";
		print(tmp);
		cout << "\n";
	}
}

int main()
{
	non_bedrock_cells = find_non_bedrock();
	play_game_simple_for_white();
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
