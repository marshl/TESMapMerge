#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <stack>
#include <string>
#include <windows.h>

const int MIN_X = -29;
const int MAX_X = 23;
const int RANGE_X = MAX_X - MIN_X;

const int MIN_Y = -19;
const int MAX_Y = 28;
const int RANGE_Y = MAX_Y - MIN_Y;

const int IMAGE_HEIGHT = 256;
const int IMAGE_WIDTH = 256;
const size_t IMAGE_ROW_SIZE = sizeof( char ) * 3 * IMAGE_WIDTH;

const int COMPOSITE_PIXEL_HEIGHT = RANGE_Y * IMAGE_HEIGHT;
const int COMPOSITE_PIXEL_WIDTH = RANGE_X * IMAGE_WIDTH;
const int COMPOSITE_BYTECOUNT = COMPOSITE_PIXEL_WIDTH * COMPOSITE_PIXEL_HEIGHT * 3;
const int COMPOSITE_BYTECOUNT_MB = COMPOSITE_BYTECOUNT / ( 1024 * 1024 );

struct ImageFile
{
    std::string filename;
    int x;
    int y;
};

void InitialiseImageCells();
void FindImageCells();
void DestroyImageCells();
void PrintFilledCells();
void RenderMap();

ImageFile*** imageCells = new ImageFile**[RANGE_Y];

const char* waterMap = "Wilderness (-21,28).bmp";

int main( int argc, char* argv[] )
{
    InitialiseImageCells();
    FindImageCells();
    PrintFilledCells();

    RenderMap();

    DestroyImageCells();
    system( "pause" );

    return 0;
}

void InitialiseImageCells()
{
    for ( int y = 0; y < RANGE_Y; ++y )
    {
        imageCells[y] = new ImageFile*[RANGE_X];
        for ( int x = 0; x < RANGE_X; ++x )
        {
            imageCells[y][x] = nullptr;
        }
    }
}

void DestroyImageCells()
{
    for ( int y = 0; y < RANGE_Y; ++y )
    {

        for ( int x = 0; x < RANGE_X; ++x )
        {
            if ( imageCells[y][x] != nullptr )
            {
                delete imageCells[y][x];
            }
        }
        delete imageCells[y];
    }
    delete imageCells;
}

void PrintFilledCells()
{
    for ( int y = RANGE_Y - 1; y >= 0; --y )
    {
        for ( int x = 0; x < RANGE_X; ++x )
        {
            std::cout << ( imageCells[y][x] != nullptr ? 'X' : ' ' );
        }

        std::cout << "\n";
    }
}

void FindImageCells()
{
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    hFind = FindFirstFile( "../maps/*", &FindFileData );

    if ( hFind == INVALID_HANDLE_VALUE )
    {
        std::cout << "Error: invalid path\n";
        return;
    }

    do
    {
        if ( strcmp( FindFileData.cFileName, "." ) == 0
            || strcmp( FindFileData.cFileName, ".." ) == 0 )
        {
            continue;
        }
        std::string filename = FindFileData.cFileName;
        std::smatch matches;
        std::regex exp( "(-?[0-9]+),(-?[0-9]+)" );
        std::regex_search( filename, matches, exp );

        if ( matches.size() < 3 )
        {
            std::cout << "Error parsing file: " << filename << "=>" << matches.size() << "\n";
            continue;
        }

        int x = atoi( matches[0].str().c_str() ) - MIN_X;
        int y = atoi( matches[2].str().c_str() ) - MIN_Y;

        if ( x >= 0 && x < RANGE_X && y >= 0 && y < RANGE_Y )
        {
            ImageFile* img = imageCells[y][x] = new ImageFile();
            img->filename = filename;
            img->x = x;
            img->y = y;
        }
    } while ( FindNextFile( hFind, &FindFileData ) != 0 );
    FindClose( hFind );
}

void RecurseiveFileLoad()
{
    std::string path = "../maps";
    std::string mask = "*";
    std::vector<std::string> files;

    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA ffd;
    std::string spec;
    std::stack<std::string> directories;

    directories.push( path );
    files.clear();

    while ( !directories.empty() )
    {
        path = directories.top();
        spec = path + "\\" + mask;
        directories.pop();
        hFind = FindFirstFile( spec.c_str(), &ffd );
        if ( hFind == INVALID_HANDLE_VALUE )
        {
            return;
        }

        do
        {
            if ( strcmp( ffd.cFileName, "." ) != 0 &&
                strcmp( ffd.cFileName, ".." ) != 0 )
            {
                if ( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
                {
                    directories.push( path + "\\" + ffd.cFileName );
                }
                else
                {
                    std::string filename = ffd.cFileName;
                    std::string fullpath = path + "\\" + filename;

                    if ( filename.rfind( '.' ) != std::string::npos )
                    {
                        std::string extensionless = filename.substr( 0, filename.rfind( '.' ) );
                        std::cout << fullpath << " -> " << extensionless << "\n";
                    }
                }
            }
        } while ( FindNextFile( hFind, &ffd ) != 0 );

        if ( GetLastError() != ERROR_NO_MORE_FILES )
        {
            FindClose( hFind );
            return;
        }

        FindClose( hFind );
        hFind = INVALID_HANDLE_VALUE;
    }
}

void RenderMap()
{
    std::ofstream fout( "../output.bmp", std::ofstream::binary );

    BITMAPFILEHEADER fileHeader;
    fileHeader.bfType = 19778;
    fileHeader.bfSize = sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) + sizeof( char ) * COMPOSITE_BYTECOUNT;
    fileHeader.bfOffBits = sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER );
    fout.write( (char*)( &fileHeader ), sizeof( BITMAPFILEHEADER ) );

    BITMAPINFOHEADER infoHeader;
    memset( &infoHeader, NULL, sizeof( BITMAPINFOHEADER ) );
    infoHeader.biSize = sizeof( BITMAPINFOHEADER );
    infoHeader.biWidth = COMPOSITE_PIXEL_WIDTH;
    infoHeader.biHeight = COMPOSITE_PIXEL_HEIGHT;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 24;
    infoHeader.biCompression = 0;
    infoHeader.biSizeImage = sizeof( char ) * COMPOSITE_BYTECOUNT;
    fout.write( (char*)( &infoHeader ), sizeof( BITMAPINFOHEADER ) );

    char dataRow[3 * IMAGE_WIDTH];

    std::ifstream rowFileHandles[RANGE_X];

    for ( int y = 0; y < RANGE_Y; ++y )
    {
        for ( int x = 0; x < RANGE_X; ++x )
        {
            std::ifstream& fin = rowFileHandles[x];
            if ( imageCells[y][x] != nullptr )
            {
                fin.open( "../maps/" + imageCells[y][x]->filename, std::ifstream::binary );
            }
            else
            {
                fin.open( "../maps/Wilderness (-21,28).bmp", std::ifstream::binary );
            }

            if ( !fin.is_open() )
            {
                std::cout << "Error opening image file: " << imageCells[y][x]->filename << "\n";
                continue;
            }

            fin.seekg( sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) );
        }
        for ( int line = 0; line < IMAGE_HEIGHT; ++line )
        {
            for ( int x = 0; x < RANGE_X; ++x )
            {
                memset( dataRow, NULL, IMAGE_ROW_SIZE );
                std::ifstream& fin = rowFileHandles[x];
                if ( fin.is_open() )
                {
                    fin.read( dataRow, IMAGE_ROW_SIZE );
                }

                fout.write( dataRow, IMAGE_ROW_SIZE );
            }
        }

        for ( int x = 0; x < RANGE_X; ++x )
        {
            if ( rowFileHandles[x].is_open() )
            {
                rowFileHandles[x].close();
            }
        }

    }
}