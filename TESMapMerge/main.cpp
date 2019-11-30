#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <stack>
#include <string>

#define NOMINMAX
#include <windows.h>

const int IMAGE_HEIGHT = 256;
const int IMAGE_WIDTH = 256;
const size_t IMAGE_ROW_SIZE = sizeof(char) * 3 * IMAGE_WIDTH;

struct ImageFile
{
	std::string filename;
	int x;
	int y;
};

void FindImageCells(int& minX, int& maxX, int& minY, int& maxY);
void PrintFilledCells();
void RenderMap(std::string fillerImagePath);

std::vector<std::vector<ImageFile*>> imageCells;

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cout << "Usage: TESMapMerge.exe filler_file\n";
		return 1;
	}

	int minX, maxX, minY, maxY;

	FindImageCells(minX, maxX, minY, maxY);
	PrintFilledCells();

	RenderMap(argv[1]);

	return 0;
}

void PrintFilledCells()
{
	for (int y = (int)imageCells.size() - 1; y >= 0; --y)
	{
		for (int x = 0; x < (int)imageCells[y].size(); ++x)
		{
			std::cout << (imageCells[y][x] != nullptr ? 'X' : ' ');
		}

		std::cout << "\n";
	}
}

void FindImageCells(int& minX, int& maxX, int& minY, int& maxY)
{
	minX = maxX = minY = maxY = 0;

	for (int i = 0; i < 2; ++i)
	{
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind = INVALID_HANDLE_VALUE;
		hFind = FindFirstFile("../maps/*", &FindFileData);

		if (hFind == INVALID_HANDLE_VALUE)
		{
			std::cout << "Error: invalid path\n";
			return;
		}

		if (i == 1)
		{
			imageCells.resize(maxY - minY + 1);
			for (int y = 0; y < (int)imageCells.size(); ++y)
			{
				imageCells[y].resize(maxX - minX + 1, nullptr);
			}
		}

		do
		{
			if (strcmp(FindFileData.cFileName, ".") == 0
				|| strcmp(FindFileData.cFileName, "..") == 0)
			{
				continue;
			}

			std::string filename = FindFileData.cFileName;
			std::smatch matches;
			std::regex exp("(-?[0-9]+),(-?[0-9]+)");
			std::regex_search(filename, matches, exp);

			if (matches.size() < 3)
			{
				std::cout << "Error parsing file: " << filename << "=>" << matches.size() << "\n";
				continue;
			}

			int xCoord = atoi(matches[0].str().c_str());
			int yCoord = atoi(matches[2].str().c_str());

			// On the first pass, find the extents
			if (i == 0)
			{
				minX = std::min(minX, xCoord);
				maxX = std::max(maxX, xCoord);
				minY = std::min(minY, yCoord);
				maxY = std::max(maxY, yCoord);
			}
			else // On the second pass, store the image files
			{
				ImageFile* img = new ImageFile();
				img->filename = filename;
				img->x = xCoord - minX;
				img->y = yCoord - minY;
				imageCells[img->y][img->x] = img;
			}
		} while (FindNextFile(hFind, &FindFileData) != 0);

		FindClose(hFind);
	}
}

void RenderMap(std::string fillerImagePath)
{
	int width = imageCells[0].size();
	int height = imageCells.size();
	std::ofstream fout("../output.bmp", std::ofstream::binary);

	int compositeByteCount = (width * IMAGE_WIDTH + height * IMAGE_HEIGHT) * sizeof(char) * 3;

	BITMAPFILEHEADER fileHeader;
	fileHeader.bfType = 19778;
	fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + compositeByteCount;
	fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	fout.write((char*)(&fileHeader), sizeof(BITMAPFILEHEADER));

	BITMAPINFOHEADER infoHeader;
	memset(&infoHeader, NULL, sizeof(BITMAPINFOHEADER));
	infoHeader.biSize = sizeof(BITMAPINFOHEADER);
	infoHeader.biWidth = width * IMAGE_WIDTH;
	infoHeader.biHeight = height * IMAGE_HEIGHT;
	infoHeader.biPlanes = 1;
	infoHeader.biBitCount = 24;
	infoHeader.biCompression = 0;
	infoHeader.biSizeImage = compositeByteCount;
	fout.write((char*)(&infoHeader), sizeof(BITMAPINFOHEADER));


	char dataRow[3 * IMAGE_WIDTH];

	std::vector<std::ifstream> rowFileHandles(width);

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			std::ifstream& fin = rowFileHandles[x];
			if (imageCells[y][x] != nullptr)
			{
				fin.open("../maps/" + imageCells[y][x]->filename, std::ifstream::binary);
			}
			else
			{
				fin.open(std::string("../maps/") + fillerImagePath, std::ifstream::binary);
			}

			if (!fin.is_open())
			{
				std::cout << "Error opening image file: " << imageCells[y][x]->filename << "\n";
				continue;
			}

			fin.seekg(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));
		}
		for (int line = 0; line < IMAGE_HEIGHT; ++line)
		{
			for (int x = 0; x < width; ++x)
			{
				memset(dataRow, NULL, IMAGE_ROW_SIZE);
				std::ifstream& fin = rowFileHandles[x];
				if (fin.is_open())
				{
					fin.read(dataRow, IMAGE_ROW_SIZE);
				}

				fout.write(dataRow, IMAGE_ROW_SIZE);
			}
		}

		for (int x = 0; x < width; ++x)
		{
			if (rowFileHandles[x].is_open())
			{
				rowFileHandles[x].close();
			}
		}

	}
}