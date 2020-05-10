#include <iostream>
#include <vector>
#include <string>
#include <Windows.h>
#include <conio.h>
#include <time.h>

using namespace std;

class coord {
public:
	int x;
	int y;
	char c;

	coord(int x, int y, char c) : coord(x, y) {
		this->c = c;
	}

	coord(int x, int y) : c('*') {
		this->x = x;
		this->y = y;
	}

	coord(coord* c) : coord(c->x, c->y) { }

	void resetCharacter() { c = '*'; }

	void setUp() { c = '^'; }
	void setRight() { c = '>'; }
	void setLeft() { c = '<'; }
	void setDown() { c = 'v'; }

	void setFood() { c = 'X'; }

	coord& operator++() { x++; return *this; }
	coord& operator--() { x--; return *this; }
	coord& operator++(int) {  x++; return *this; }
	coord& operator--(int) { x--; return *this; }
	coord& operator+=(int i) { y++; return *this; }
	coord& operator-=(int i) { y--; return *this; }

	bool operator==(coord& c) { return c.x == (*this).x && c.y == (*this).y; }
};

class screen {
private:
	HANDLE hConsoleOutput;
	CONSOLE_CURSOR_INFO oldCursorInfo, curCursorInfo;
	WORD oldTextAttr;

public:
	screen() {
		hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		GetConsoleCursorInfo(hConsoleOutput, &oldCursorInfo);
		curCursorInfo.dwSize = oldCursorInfo.dwSize;
		curCursorInfo.bVisible = oldCursorInfo.bVisible;
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(hConsoleOutput, &csbi);
		oldTextAttr = csbi.wAttributes;
	}

	void drawCoord(coord cor) {
		drawCharacter(cor, cor.c);
	}

	void drawCharacter(coord cor, char c) {
		drawCharacter(cor.x, cor.y, c);
	}

	void drawCharacter(int x, int y, char c = 0) {
		COORD point;
		point.X = static_cast<SHORT>(x);
		point.Y = static_cast<SHORT>(y);
		SetConsoleCursorPosition(hConsoleOutput, point);
		if (c > 0)
			_putch(c);
	}

	void clearScreen() {
		system("cls");
	}

	void cursorShow(bool visible) {
		curCursorInfo.bVisible = visible;
		SetConsoleCursorInfo(hConsoleOutput, &curCursorInfo);
	}
};

class snake {
private:
	vector<coord> _snake;

public:
	snake() {}
	snake(int headX, int headY, int size) {
		_snake.push_back(coord(headX, headY, '>'));
		for (int i = 1; i < size; i++)
			_snake.push_back(coord(headX - i, headY));
	}

	void firstDraw(screen* s) {
		for (auto it = _snake.begin(); it != _snake.end(); it++)
			s->drawCharacter(*it, (*it).c);
	}

	void insert(coord& c) {
		_snake.insert(++_snake.begin(), c);
	}

	snake& operator++() {
		auto newC = coord(_snake[0]);
		newC.resetCharacter();
		_snake[0]++;
		_snake.insert(++_snake.begin(), newC);
		_snake.pop_back();
		return *this;
	}

	snake& operator--() {
		auto newC = coord(_snake[0]);
		newC.resetCharacter();
		_snake[0]--;
		_snake.insert(++_snake.begin(), newC);
		_snake.pop_back();
		return *this;
	}

	snake& operator++(int) {
		return ++(*this);
	}

	snake& operator--(int) {
		return --(*this);
	}

	snake& operator+=(int i) {
		auto newC = coord(_snake[0]);
		newC.resetCharacter();
		_snake[0] += 1;
		_snake.insert(++_snake.begin(), newC);
		_snake.pop_back();
		return *this;
	}

	snake& operator-=(int i) {
		auto newC = coord(_snake[0]);
		newC.resetCharacter();
		_snake[0] -= 1;
		_snake.insert(++_snake.begin(), newC);
		_snake.pop_back();
		return *this;
	}

	coord& operator[](int index) {
		return _snake[index];
	}

	int size() { return _snake.size(); }

	bool into(coord& c, bool includeHead = true) {
		auto it = _snake.begin();
		if (!includeHead)
			it++;
		for (; it < _snake.end(); it++)
			if (*it == c)
				return true;
		return false;
	}
};

class game {
private:
	int width;
	int height;
	int difficulty;
	snake s;
	screen scr;
	enum Command { CMD_NOCOMMAND = 0, CMD_EXIT, CMD_LEFT, CMD_RIGHT, CMD_UP, CMD_DOWN };
	enum State { STATE_OK, STATE_EXIT, STATE_DIED };
	typedef pair<int, Command> CmdPair;
	CmdPair cmd_table[5];
	State state;
	double durationGame;
public:
	game() {
		s = snake(6, 3, 4);
		scr.cursorShow(false);

		cmd_table[0] = CmdPair(27, CMD_EXIT);
		cmd_table[1] = CmdPair('K', CMD_LEFT);
		cmd_table[2] = CmdPair('M', CMD_RIGHT);
		cmd_table[3] = CmdPair('H', CMD_UP);
		cmd_table[4] = CmdPair('P', CMD_DOWN);
	}

	void startEasy() {
		width = 200;
		height = 60;
		difficulty = 200;
		start();
	}

	void startMedium() {
		width = 100;
		height = 30;
		difficulty = 100;
		start();
	}

	void startHard() {
		width = 50;
		height = 20;
		difficulty = 70;
		start();
	}

private:
	void start() {
		firstDraw();
		gameLoop();
	}

	void firstDraw() {
		scr.clearScreen();
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				if (j == 0 || j == height - 1) {
					scr.drawCharacter(i, j, '#');
				}
				if (i == 0 || i == width - 1) {
					scr.drawCharacter(i, j, '#');
				}
			}
		}
		s.firstDraw(&scr);
	}

	void gameLoop() {
		auto lastCommand = Command::CMD_RIGHT;
		auto food = makeFood();
		scr.drawCoord(food);
		clock_t time1, time2, duration;
		time1 = clock();
		do {	
			auto tempCommand = lastCommand;
			if (_kbhit())
				tempCommand = getCommand();

			switch (tempCommand)
			{
			case Command::CMD_UP:
				if (lastCommand != Command::CMD_DOWN)
					lastCommand = tempCommand;
				break;
			case Command::CMD_DOWN:
				if (lastCommand != Command::CMD_UP)
					lastCommand = tempCommand;
				break;
			case Command::CMD_LEFT:
				if (lastCommand != Command::CMD_RIGHT)
					lastCommand = tempCommand;
				break;
			case Command::CMD_RIGHT:
				if (lastCommand != Command::CMD_LEFT)
					lastCommand = tempCommand;
				break;
			case Command::CMD_EXIT:
				lastCommand = tempCommand;
			default:
				break;
			}

			auto tail = s[s.size() - 1];

			switch (lastCommand)
			{
			case Command::CMD_UP:
				s -= 1;
				s[0].setUp();
				break;
			case Command::CMD_DOWN:
				s += 1;
				s[0].setDown();
				break;
			case Command::CMD_LEFT:
				s--;
				s[0].setLeft();
				break;
			case Command::CMD_RIGHT:
				s++;
				s[0].setRight();
				break;
			case Command::CMD_EXIT:
				state = State::STATE_EXIT;
			default:
				break;
			}

			scr.drawCharacter(tail, ' ');
			scr.drawCoord(s[0]);
			scr.drawCoord(s[1]);
			
			auto head = s[0];
			if (head.x == 0 || head.x == width - 1 || head.y == 0 || head.y == height - 1 || s.into(head, false))
				state = State::STATE_DIED;
			
			if (state == State::STATE_OK) {
				if (head == food) {
					s.insert(food);
					food = makeFood();
					scr.drawCoord(food);
				}
			}

			time2 = clock();
			duration = time2 - time1;
			auto d = static_cast<double>(duration) / CLOCKS_PER_SEC;
			durationGame = d;

			printStat();

			Sleep(difficulty);

		} while (state == State::STATE_OK);

		if (state == State::STATE_DIED) {
			scr.drawCharacter(width / 2 - 12, height / 2);
			cout << "Y O U R  A R E  D I E D";
		}

		if (state == State::STATE_EXIT) {
			scr.drawCharacter(width / 2 - 8, height / 2);
			cout << "G A M E  O V E R";
			scr.drawCharacter(width / 2 - 7, height / 2 + 1);
			cout << "G O O D  B Y E";
		}

		scr.drawCharacter(0, height + 20);
	}

	Command getCommand() {
		int ch;
		ch = _getch();
		if (ch == 0 || ch == 0xe0) {
			ch = _getch();
		}

		for (int i = 0; i < 5; i++) {
			if (cmd_table[i].first == ch) {
				return cmd_table[i].second;
			}
		}
		return CMD_NOCOMMAND;
	}

	void printStat() {
		scr.drawCharacter(3, height);
		cout << "size: ";
		_cprintf("%04u", s.size());
		scr.drawCharacter(22, height);
		cout << "time: ";
		_cprintf("%07.2f", durationGame);
	}

	coord makeFood() {
		coord food(0, 0);
		do {
			food.x = rand() % (width - 2) + 1;
			food.y = rand() % (height - 2) + 1;
		} while (s.into(food));

		food.setFood();
		return food;
	}
};

int main()
{
	SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, 0);

	cout << "choose difficulty level:" << endl
		<< "\t1. easy" << endl
		<< "\t2. medium" << endl
		<< "\t3. hard" << endl
		<< "result: ";

	game g;
	int r;
	cin >> r;
	if (r == 1)
		g.startEasy();
	else if (r == 2)
		g.startMedium();
	else if (r == 3)
		g.startHard();

	return 0;
}
