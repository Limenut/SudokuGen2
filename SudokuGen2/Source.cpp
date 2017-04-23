#include <iostream>
#include <vector>
#include <array>
#include <cstdlib>
#include <time.h>
#include <bitset>

unsigned short numberToBit(uint8_t n)
{
	return 1 << (n - 1);
}

uint8_t bitToNumber(unsigned short)
{
	return 0;
}

uint8_t getRow(uint8_t i)
{
	return i / 9;
}

uint8_t getColumn(uint8_t i)
{
	return i % 9;
}

uint8_t getSection(uint8_t row, uint8_t column)
{
	return ((row / 3) * 3) + (column / 3);
}

uint8_t randomize(std::vector<uint8_t> &pool)
{
	if (pool.size())
	{
		uint8_t i = uint8_t(rand() % pool.size());
		uint8_t chosen = pool[i];
		pool.erase(pool.begin() + i);
		return chosen;
	}
	else return 0;
	//return pool.size() ? pool[rand() % pool.size()] : 0;
}


struct Square
{
	bool isReserved = false;
	uint8_t number = 0;
	unsigned short available_nums = 0b111111111;

	void reset()
	{
		number = 0;
		available_nums = 0b111111111;
	}
};

struct Sudoku
{
	std::array<Square, 81> squares;

	unsigned short pool_rows[9];
	unsigned short pool_columns[9];
	unsigned short pool_sections[9];

	Sudoku();
	uint8_t chooseNumber(unsigned short& row, unsigned short& column, unsigned short& section, unsigned short& total);
	void generate();
	void generateClues(uint8_t clueCount);
	bool solve(uint8_t begin);
	bool checkUniqueness();
	void resetPools();
	void poolsFromClues();
	void print();
	void printClues();
	void clearUnkown();
};

Sudoku::Sudoku()
{
	for (uint8_t i = 0; i < 9; i++)
	{
		pool_rows[i] = 0b111111111;
		pool_columns[i] = 0b111111111;
		pool_sections[i] = 0b111111111;
	}
}

uint8_t Sudoku::chooseNumber(unsigned short& row, unsigned short& column, unsigned short& section, unsigned short& total)
{
	unsigned short binPool = row & column & section & total;	//only numbers which fit each category*/
	std::vector<uint8_t> pool;
	unsigned short num = 1;

	//populate number array with suitable numbers. num checks a bit at a time
	for (uint8_t i = 1; i < 10; i++)
	{
		if (binPool & num) pool.push_back(i);
		num = num << 1;
	}

	//choose random number of available ones
	uint8_t chosen = randomize(pool);

	if (chosen)
	{
		num = 1 << (chosen - 1); //convert number back to matching bit in array

		//make chosen bit unavailable
		row = row & ~num;
		column = column & ~num;
		section = section & ~num;
		total = total & ~num;
	}

	return chosen;
}

void Sudoku::generate()
{

	//for every square of the grid; bruteforce a solution with backtracking
	for (uint8_t i = 0; i < 9 * 9;)
	{
		uint8_t row = i / 9;
		uint8_t column = i % 9;
		uint8_t section = ((row / 3) * 3) + (column / 3);

		//pass numbers available to this square
		uint8_t num = chooseNumber(pool_rows[row], pool_columns[column], pool_sections[section], squares[i].available_nums);

		//std::cout << (int)i << "  " << (int)num << "  " << std::bitset<9>(pool_rows[row] & pool_columns[column] & pool_sections[section]) << "  " << std::bitset<9>(squares[i].available_nums) << std::endl;

		if (num == 0)	//no numbers were possible, goto previous square
		{

			squares[i].reset();
			i--;

			uint8_t row = i / 9;
			uint8_t column = i % 9;
			uint8_t section = ((row / 3) * 3) + (column / 3);

			unsigned short bit = 1 << (squares[i].number - 1); //convert number back to matching bit in array

															   //remove previous square's limitations
			pool_rows[row] = pool_rows[row] | bit;
			pool_columns[column] = pool_columns[column] | bit;
			pool_sections[section] = pool_sections[section] | bit;
		}
		else
		{
			squares[i].number = num;
			i++;
		}
	}

	//cleanup
	resetPools();
}

void Sudoku::generateClues(uint8_t clueCount)
{
	std::vector<uint8_t> indices;

	for (uint8_t i = 0; i < 9 * 9; i++)
	{
		indices.push_back(i);
	}

	for (uint8_t i = 0; i < clueCount; i++)
	{
		uint8_t in = randomize(indices);
		squares[in].isReserved = true;

		unsigned short bit = numberToBit(squares[in].number);

		uint8_t row = getRow(in);
		uint8_t column = getColumn(in);
		uint8_t section = getSection(row, column);

		pool_rows[row] = pool_rows[row] & ~bit;
		pool_columns[column] = pool_columns[column] & ~bit;
		pool_sections[section] = pool_sections[section] & ~bit;
	}

	//clear unknown numbers
	clearUnkown();
}

bool Sudoku::solve(uint8_t begin)
{
	//for every square of the grid; bruteforce a solution with backtracking
	for (uint8_t i = begin; i < 9 * 9;)
	{
		//skip reserved squares (don't need to check here?)
		if (squares[i].isReserved)
		{
			i++;
			continue;
		}

		uint8_t row = i / 9;
		uint8_t column = i % 9;
		uint8_t section = ((row / 3) * 3) + (column / 3);

		//pass numbers available to this square
		uint8_t num = chooseNumber(pool_rows[row], pool_columns[column], pool_sections[section], squares[i].available_nums);

		if (num == 0)	//no numbers were possible, goto previous square
		{
			//every single configuration is exhausted
			if (i == 0) return false;

			squares[i].reset();
			
				
			//forward = false;
			
			do
			{
				i--;
				if (i < 0) return false;
			} while (squares[i].isReserved);

			uint8_t row = i / 9;
			uint8_t column = i % 9;
			uint8_t section = ((row / 3) * 3) + (column / 3);

			unsigned short bit = 1 << (squares[i].number - 1); //convert number back to matching bit in array

			//remove previous square's limitations
			pool_rows[row] = pool_rows[row] | bit;
			pool_columns[column] = pool_columns[column] | bit;
			pool_sections[section] = pool_sections[section] | bit;

			
			
		}
		else
		{		
			squares[i].number = num;
			i++;
			//forward = true;
		}
		//std::cout << std::endl << std::endl << (int)i;
		//print();
	}

	/*for (uint8_t i = 0; i < 9; i++)
	{
		pool_rows[i] = 0b111111111;
		pool_columns[i] = 0b111111111;
		pool_sections[i] = 0b111111111;
	}
	resetPools();
	clearUnkown();
	poolsFromClues();*/
	return true;
}

void Sudoku::resetPools()
{
	for (uint8_t i = 0; i < 9 * 9; i++)
	{
		squares[i].available_nums = 0b111111111;
	}

	for (uint8_t i = 0; i < 9; i++)
	{
		pool_rows[i] = 0b111111111;
		pool_columns[i] = 0b111111111;
		pool_sections[i] = 0b111111111;
	}
}

void Sudoku::poolsFromClues()
{
	for (uint8_t i = 0; i < 9*9; i++)
	{
		if (squares[i].isReserved)
		{
			unsigned short bit = numberToBit(squares[i].number);

			uint8_t row = getRow(i);
			uint8_t column = getColumn(i);
			uint8_t section = getSection(row, column);

			pool_rows[row] = pool_rows[row] & ~bit;
			pool_columns[column] = pool_columns[column] & ~bit;
			pool_sections[section] = pool_sections[section] & ~bit;
		}	
	}
}

void Sudoku::print()
{
	for (uint8_t i = 0; i < 9 * 9; i++)
	{
		if (!(i % 9)) std::cout << std::endl;
		std::cout << (int)squares[i].number << " ";
	}
	std::cout << std::endl;
}

void Sudoku::printClues()
{
	for (uint8_t i = 0; i < 9 * 9; i++)
	{
		if (!(i % 9)) std::cout << std::endl;

		if (squares[i].isReserved)
		{
			std::cout << (int)squares[i].number << " ";
		}
		else
		{
			std::cout << "* ";
		}
	}
	std::cout << std::endl;
}

bool Sudoku::checkUniqueness()
{
	solve(0);
	if (solve(0)) return false;
	else return true;
}

void Sudoku::clearUnkown()
{
	for (uint8_t i = 0; i < 9 * 9; i++)
	{
		if (!squares[i].isReserved) squares[i].number = 0;
	}
}

int main()
{
	//srand((unsigned)time(NULL));

	Sudoku sudo;

	sudo.generate();
	//sudo.solve();
	sudo.print();

	sudo.generateClues(79);
	sudo.printClues();

	for (uint8_t i = 0; i < 9; i++)
	{
		//std::cout << std::endl << std::bitset<9>(sudo.pool_rows[i]);
	}

	std::cout << std::endl << sudo.solve(0);
	std::cout << std::endl << sudo.solve(81);
	//std::cout << std::endl << sudo.checkUniqueness();
	//sudo.print();
}