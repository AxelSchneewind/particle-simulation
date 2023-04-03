#include "Stack.hpp"
#include "Vector.hpp"

typedef Vector<2> Vec;

template<typename T>
class UniformGrid {
private:
    Vec center;
    double size;
    Stack<T>* cells;
    int* neighborCells;

    int index(Vec position);
    int index(int x, int y);
    int x(int index);
    int y(int index);

public:
    int cellNeighborCount;
    int cellNeighborWidth;
    int objCounter;
    int cellsPerDimension;
    int cellCapacity;

    UniformGrid(Vec center, double size, int cellsPerDimension, int cellCapacity, int cellNeighborWidth);

    
    Stack<T>* cell(Vec position);
    Stack<T>* cell(int x, int y);
    Stack<T>* cell(int idx);

    int* neighboringCells(Vec position);

    void insert(T obj, Vec position);
    void remove(T obj, Vec position);
    void clear();
};



template <typename T>
int UniformGrid<T>::index(Vec position)
{
    int x = (int)round((position[0] - center[0] + size/2) * cellsPerDimension / size);
    int y = (int)round((position[1] - center[1] + size/2) * cellsPerDimension / size);

    x = x < 0 ? 0 : x;
    y = y < 0 ? 0 : y;
    x = x >= cellsPerDimension ? cellsPerDimension - 1 : x;
    y = y >= cellsPerDimension ? cellsPerDimension - 1 : y;

    return index(x, y);
}

template <typename T>
int UniformGrid<T>::index(int x, int y)
{
    return y * cellsPerDimension + x;
}

template <typename T>
int UniformGrid<T>::x(int index)
{
    return index % cellsPerDimension;
}
template <typename T>
int UniformGrid<T>::y(int index)
{
    return index / cellsPerDimension;
}

template <typename T>
UniformGrid<T>::UniformGrid(Vec center, double size, int cellsPerDimension, int cellCapacity, int cellNeighborWidth) : 
    objCounter(0), 
    cellsPerDimension(cellsPerDimension), 
    cellNeighborCount(((2 * cellNeighborWidth + 1) * (2 * cellNeighborWidth + 1))), 
    cellNeighborWidth(cellNeighborWidth),
    size(size), 
    cells(new Stack<T>[cellsPerDimension * cellsPerDimension]), 
    neighborCells(new int[(cellsPerDimension * cellsPerDimension) * ((2 * cellNeighborWidth + 1) * (2 * cellNeighborWidth + 1))])
{
    for (int i = 0; i < cellsPerDimension * cellsPerDimension; i++)
    {
        cells[i].alloc(cellCapacity);

        int X = x(i);
        int Y = y(i);
        int neighborIdx = 0;
        for (int j = -cellNeighborWidth; j <= cellNeighborWidth; j++)
        {
            for (int k = -cellNeighborWidth; k <= cellNeighborWidth; k++)
            {
                if (0 <= X + j && X + j < cellsPerDimension && 0 <= Y + k && Y + k < cellsPerDimension)
                {
                    neighborCells[i * cellNeighborCount + neighborIdx] = index(X + j, Y + k);
                }
                else
                {
                    neighborCells[i * cellNeighborCount + neighborIdx] = -1;
                }
                neighborIdx++;
            }
        }
    }
}

template <typename T>
Stack<T> *UniformGrid<T>::cell(Vec position)
{
    return &cells[index(position)];
}

template <typename T>
Stack<T> *UniformGrid<T>::cell(int x, int y)
{
    return &cells[index(x, y)];
}

template <typename T>
Stack<T> *UniformGrid<T>::cell(int idx)
{
    return &cells[idx];
}

template <typename T>
int *UniformGrid<T>::neighboringCells(Vec position)
{
    return &neighborCells[index(position) * cellNeighborCount];
}

template <typename T>
void UniformGrid<T>::insert(T obj, Vec position)
{
    objCounter++;
    cells[index(position)].push(obj);
}

template <typename T>
void UniformGrid<T>::remove(T obj, Vec position)
{
    objCounter--;
    cells[index(position)].remove(obj);
}

template <typename T>
void UniformGrid<T>::clear()
{
    objCounter = 0;
    for (int i = 0; i < cellsPerDimension * cellsPerDimension; i++)
    {
        cells[i].clear();
    }
}


template <typename T>
std::ostream &operator<<(std::ostream &os, UniformGrid<T> *&grid)
{
    os << "{ " << grid->objCounter << std::endl;
    for (int i = 0; i < grid->cellsPerDimension; i++)
    {
        os << "\t";
    for (int j = 0; j < grid->cellsPerDimension - 1; j++)
    {
        os << *grid->cell(i,j) << ", ";
    }
    os << *grid->cell(i, grid->cellsPerDimension - 1) << std::endl;

    }
    return os << std::endl << "}" << std::endl;
}

template <typename T>
std::ostream &operator<<(std::ostream &os, UniformGrid<T> &grid)
{
    os << "{ " << grid.objCounter << std::endl;
    for (int i = 0; i < grid.cellsPerDimension; i++)
    {
        os << "\t";
    for (int j = 0; j < grid.cellsPerDimension - 1; j++)
    {
        os << *grid.cell(i,j) << ", ";
    }
    os << *grid.cell(i, grid.cellsPerDimension - 1) << std::endl;

    }
    return os << std::endl << "}" << std::endl;
}



