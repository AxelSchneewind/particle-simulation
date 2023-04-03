#pragma once
#include <ostream>
#include <cmath>



template<int M, int N>
class Matrix;

template<int M>
class Vector { 
private:
    double v[M]; 
public:
    Vector();
    Vector(double values[M]);
    Vector(std::initializer_list<double> c);

    double& operator[](int index);
    const double& operator[](int index) const;
    Vector<M> operator*(double scalar) const;

    Matrix<M,M> operator*(Matrix<1,M> other) const;

    Vector<M> operator+(Vector<M> other) const;
    Vector<M> operator-(Vector<M> other) const;

    Vector<M>& operator+=(const Vector<M>& rhs);

    bool operator==(Vector<M>  other) const;
    bool operator!=(Vector<M>  other) const;
    Matrix<1,M> transpose() const;
    double sqrLength() const;
    double length() const;
};


template<int M, int N>
class Matrix{
private:
    double v[M][N]; 
public: 
    Matrix();
    Matrix(double values[M]);


    Vector<M>& operator[](int index);
    const Vector<M>& operator[](int index) const;
    Matrix<M,N> operator*(double scalar) const;

    Matrix<M,M> operator*(Matrix<N,M> other) const;

    Vector<M> operator*(Vector<M> other) const;
    Matrix<M,N> operator+(Matrix<M,N> other) const;
    Matrix<M,N> operator-(Matrix<M,N> other) const;
};

template<int M>
Vector<M> zeros();

/*template<int M, int N>
inline Matrix<M,N> zeros() {
    Matrix<M,N> result;
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < N; j++)
        {
            result[i][j] = 0.0;
        }
    }
    return result;
}*/

template<int M>
Matrix<M,M> identity();

template<int N>
std::ostream& operator<<(std::ostream& os, const Vector<N>& vec);

template<int M, int N>
std::ostream& operator<<(std::ostream& os, const Matrix<M,N>& mat);





template <int M>
inline double &Vector<M>::operator[](int index)
{
    return v[index];
};

template <int M>
inline const double &Vector<M>::operator[](int index) const
{
    return v[index];
};

template <int M, int N>
inline Vector<M> &Matrix<M, N>::operator[](int index)
{
    return *(Vector<M> *)v[index];
};

template <int M>
Vector<M>::Vector() {}

template<int M>
Vector<M>::Vector(std::initializer_list<double> c) {
    std::copy(c.begin(), c.end(), v);
}

template <int M>
Vector<M>::Vector(double values[M])
{
    for (int i = 0; i < M; i++)
    {
        v[i] = values[i];
    }
}

template <int M, int N>
Matrix<M, N>::Matrix(double values[M])
{
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < N; j++)
        {
            v[i][j] = values[i * N + j];
        }
    }
}

template <int M>
inline double Vector<M>::sqrLength() const { return v[0] * v[0] + v[1] * v[1]; }

template <int M>
inline double Vector<M>::length() const { return sqrt(sqrLength()); }

template <int M>
inline Matrix<1, M> Vector<M>::transpose() const
{
    Matrix<1, M> result;
    for (int i = 0; i < M; i++)
    {
        result[0][i] = v[i];
    }
    return result;
}

template <int M>
inline Vector<M> Vector<M>::operator*(double scalar) const
{
    Vector<M> result;
    result[0] = v[0] * scalar;
    result[1] = v[1] * scalar;
    return result;
}

template <int M>
inline Matrix<M, M> Vector<M>::operator*(Matrix<1, M> other) const
{
    Matrix<M, M> result;
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < M; j++)
        {
            result[i][j] = v[i] * other[0][j];
        }
    }
    return result;
}

template <int M>
inline bool Vector<M>::operator==(Vector<M> other) const
{
    for (int i = 0; i < M; i++)
    {
        if (v[i] != other[i])
            return false;
    }
    return true;
}
template <int M>
bool Vector<M>::operator!=(Vector<M> other) const { return !operator==(other); }

template <int M>
inline Vector<M> &Vector<M>::operator+=(const Vector<M> &rhs)
{
    for (int i = 0; i < M; i++)
    {
        v[i] += rhs[i];
    }
    return *this;
}

template <int M>
inline Vector<M> Vector<M>::operator+(Vector<M> v1) const
{
    Vector<M> result;
    for (int i = 0; i < M; i++)
    {
        result[i] = v[i] + v1[i];
    }

    return result;
}
template <int M>
inline Vector<M> Vector<M>::operator-(Vector<M> v1) const
{
    Vector<M> result;
    result[0] = v[0] - v1[0];
    result[1] = v[1] - v1[1];
    return result;
}

template <int M, int N>
inline Vector<M> Matrix<M, N>::operator*(Vector<M> vec) const
{
    Vector<M> result;
    result[0] = v[0][0] * vec[0] + v[0][1] * vec[1];
    result[1] = v[1][0] * vec[0] + v[1][1] * vec[1];
    return result;
}

template <int M, int N>
Matrix<M, N>::Matrix() {}

template <int M, int N>
Matrix<M, M> Matrix<M, N>::operator*(Matrix<N, M> other) const
{
    Matrix<M, M> result;
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < M; j++)
        {
            result[i][j] = 0.0;
            for (int l = 0; l < N; l++)
            {
                result[i][j] += v[i][l] * other[l][j];
            }
        }
    }
    return result;
}

template <int M, int N>
Matrix<M, N> Matrix<M, N>::operator*(double scalar) const
{
    Matrix<M, N> result;
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < N; j++)
        {
            result[i][j] *= scalar;
        }
    }
    return result;
}

template <int M, int N>
Matrix<M, N> Matrix<M, N>::operator+(Matrix<M, N> other) const
{
    Matrix<M, N> result;
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < N; j++)
        {
            result[i][j] = v[i][j] + other[i][j];
        }
    }
    return result;
}
template <int M, int N>
Matrix<M, N> Matrix<M, N>::operator-(Matrix<M, N> other) const
{
    Matrix<M, N> result;
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < N; j++)
        {
            result[i][j] = v[i][j] - other[i][j];
        }
    }
    return result;
}

template <int M>
inline Vector<M> zeros()
{
    Vector<M> result;
    for (int i = 0; i < M; i++)
    {
        result[i] = 0.0;
    }
    return result;
}

/*template<int M, int N>
inline Matrix<M,N> zeros() {
    Matrix<M,N> result;
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < N; j++)
        {
            result[i][j] = 0.0;
        }
    }
    return result;
}*/

template <int M>
inline Matrix<M, M> identity()
{
    Matrix<M, M> result;
    for (int i = 0; i < M; i++)
    {
        result[i][i] = 1.0;
        for (int j = 0; j < M && j != i; j++)
        {
            result[i][j] = 0.0;
        }
    }
    return result;
}

template <int N>
std::ostream &operator<<(std::ostream &os, const Vector<N> &vec)
{
    os << '(';
    for (int i = 0; i < N - 1; i++)
    {
        os << vec[i] << ',';
    }
    return os << vec[N - 1] << ')';
}

template <int M, int N>
std::ostream &operator<<(std::ostream &os, const Matrix<M, N> &mat)
{
    os << '(';
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < N; j++)
        {
            os << mat[i][j] << ',';
        }
    }
    return os << ')';
}
