#include <iostream>
#include <vector>
#include <array>
#include <cstdlib>
#include <time.h>
//#include <bitset>

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

uint8_t randomize(std::vector<uint8_t> pool)
{
	return pool.size() ? pool[rand() % pool.size()] : 0;
}

uint8_t chooseNumber(unsigned short& row, unsigned short& column, unsigned short& section, unsigned short& total)
{
	unsigned short binPool = row & column & section & total;	//only numbers which fit each category
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

std::array<Square, 81> generateSudoku()
{
	srand((unsigned)time(NULL));

	std::array<Square, 81> squares;

	//arrays of bit arrays; lists of numbers not yet used
	unsigned short pool_rows[9];
	unsigned short pool_columns[9];
	unsigned short pool_sections[9];

	for (uint8_t i = 0; i < 9; i++)
	{
		pool_rows[i] = 0b111111111;
		pool_columns[i] = 0b111111111;
		pool_sections[i] = 0b111111111;
	}

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

			//squares[i].number = 0;
		}
		else
		{
			squares[i].number = num;
			i++;
		}
	}
	
	return squares;
}

int main()
{
	std::array<Square, 81> squares = generateSudoku();

	for (uint8_t i = 0; i < 9 * 9; i++)
	{
		if (!(i % 9)) std::cout << std::endl;
		std::cout << (int)squares[i].number << " ";
	}
}