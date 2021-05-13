#include <iostream>
#include <intrin.h>
#include <random>
#include <ctime>
#define N 4
#define MAX_RANDOM 20

/*
* ВАРИАНТ 11
*
* Размерность матриц 4х4. Уменьшить все элементы матрицы М1 в 2 число раз. Умножить М1 на матрицу М2.
* Сложить поэлементно первую и вторую строки матрицы.
*/

typedef float Vector[N];
typedef Vector QuadraticMatrix[N];


std::ostream& operator<<(std::ostream& out, const QuadraticMatrix& matrix) {
	out.precision(4);
	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < N; ++j) {
			out << matrix[i][j] << '\t';
		}
		out << std::endl;
	}
	return out;
}

void generateQuadraticMatrix(QuadraticMatrix& matrix) {
	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < N; ++j) {
			matrix[i][j] = rand() % MAX_RANDOM;
		}
	}
}

void printResults(bool isSSE, QuadraticMatrix& result1, QuadraticMatrix& result2, QuadraticMatrix& result3, __int64 tiсks) {
	std::cout << (isSSE ? "" : "NO ") << "SSE TEST RESULTS:" << std::endl << std::endl;
	std::cout << "M1 / 2" << ":" << std::endl << result1 << std::endl;
	std::cout << "M1 * M2:" << std::endl << result2 << std::endl;
	std::cout << "M1 + M1 * M2:" << std::endl << result3 << std::endl;
	std::cout << "Time spent: " << ticks << " ticks" << std::endl;
}


int main() {
	unsigned __int64 start;
	unsigned __int64 end;
	QuadraticMatrix m1;
	QuadraticMatrix	m2;
	QuadraticMatrix result1;
	QuadraticMatrix result2;
	QuadraticMatrix result3;
	QuadraticMatrix resultSSE1;
	QuadraticMatrix resultSSE2;
	QuadraticMatrix	resultSSE3;
	Vector column;
	Vector vector;
	float m = 2;

	srand(time(nullptr));
	generateQuadraticMatrix(m1);
	generateQuadraticMatrix(m2);

	std::cout << "M1:" << std::endl << m1 << std::endl;
	std::cout << "M2:" << std::endl << m2 << std::endl;

	/*
	* В результате NO SSE ассемблерной вставки будет:
	* - в result1 будет храниться M1 / 2;
	* - в result2 будет храниться M1 * M2 (результат легко проверить на https://planetcalc.ru/1208);
	*/
	{
		start = __rdtsc();
		_asm {
			finit                                             // инициализация сопроцессора

		// получение матрицы result1
		mov    ecx, N* N
		mov    eax, 0                                     // пусть eax будет индексом k-х элементов матриц

		REDUCING_NO_SSE:
			fld[m1 + 4 * eax]                             // кладем в стек k-й элемент M1
				fld    m                                          // кладем в стек m
				fdivp  st(1), st(0)                               // делим k-й элемент m1 на m
				fstp[result1 + 4 * eax]                        // выталкиваем полученное частное в k-й элемент result1
				inc    eax
				loop   REDUCING_NO_SSE

				// получение матрицы result2
				mov    ecx, N
				xor eax, eax                                   // пусть eax будет индексом элемента k в M1
				xor ebx, ebx                                   // пусть ebx будет номером i-й строки M1

				COMPOSITING_NO_SSE :
			xor edx, edx                                   // пусть edx будет номером j-го столбца M2

				GETTING_ELEMENT_NO_SSE :
			// умножаем i-ю строку и j-й столбец поэлементно
			fld[m1 + 4 * ebx]
				fld[m2 + 4 * edx]
				fmulp  st(1), st(0)
				fld[m1 + 4 * ebx + 4]
				fld[m2 + 4 * edx + 16]
				fmulp  st(1), st(0)
				fld[m1 + 4 * ebx + 8]
				fld[m2 + 4 * edx + 32]
				fmulp  st(1), st(0)
				fld[m1 + 4 * ebx + 12]
				fld[m2 + 4 * edx + 48]
				fmulp  st(1), st(0)

				// сохраняем сумму 4-х последних элементов в стеке
				faddp  st(1), st(0)
				faddp  st(1), st(0)
				faddp  st(1), st(0)
				fstp[result2 + 4 * eax]                        // сохраняем сумму в k-й элемент result2

				// повторяем всё это для каждого элемента строки
				inc    eax
				inc    edx
				cmp    edx, N
				jne    GETTING_ELEMENT_NO_SSE

				// повторяем всё это для каждой строки
				add    ebx, 4
				loop   COMPOSITING_NO_SSE

				// получение матрицы result3
				mov    ecx, N* N
				mov    eax, 0

				// копия матрицы result2 в result3
				COPY_NO_SSE:
			fld[result2 + 4 * eax]
				fstp[result3 + 4 * eax]
				inc    eax
				loop   COPY_NO_SSE

				mov    ecx, N
				mov    eax, 0

				ADDING_1_AND_2_NO_SSE:
			// кладем в стек k-й элементы 1-й и 2-й строк матрицы result3
			fld[result3 + 4 * eax]
				fld[result3 + 4 * eax + 16]
				faddp  st(1), st(0)
				fstp[result3 + 4 * eax]
				inc    eax
				loop   ADDING_1_AND_2_NO_SSE
		}
		end = __rdtsc();

		// результаты теста NO SSE
		std::cout << std::endl << std::endl;
		printResults(false, result1, result2, result3, end - start);
	}

	/*
	* В результате SSE ассемблерной вставки будет:
	* - в resultSSE1 будет храниться M1 / m;
	* - в resultSSE2 будет храниться M1 * M2 (результат легко проверить на https://planetcalc.ru/1208).
	*/
	{
		start = __rdtsc();
		_asm {
			finit                                             // инициализация сопроцессора

		// получение вектора из N заданных чисел m
		mov    ecx, N
		xor eax, eax

		MAKING_VECTOR :
			fld    m
				fstp[vector + eax]
				add    eax, 4
				loop   MAKING_VECTOR

				// получение матрицы resultSSE1
				mov    ecx, N
				xor eax, eax                                   // пусть eax будет индексом 1-го элемента в i-й строке M1

				REDUCING_SSE :
				movups xmm0, [m1 + 4 * eax]                       // копируем в xmm0 i-ю строку матрицы M1
				movups xmm1, [vector]                             // копируем в xmm1 вектор, состоящий из N заданных чисел m
				divps  xmm0, xmm1                                 // уменьшаем в m раз i-ю строку M1
				movups[resultSSE1 + 4 * eax], xmm0               // вставляем уже уменьшенную в m раз строку в resultSSE1
				add    eax, 4
				loop   REDUCING_SSE

				// получение матрицы resultSSE2
				mov    ecx, N
				xor eax, eax                                   // пусть eax будет индексом элемента k в M1
				xor ebx, ebx                                   // пусть ebx будет индексом 1-го элемента в i-й строке M1

				COMPOSITING_SSE :
			xor edx, edx                                   // пусть edx будет номером j-го столбца M2

				GETTING_ELEMENT_SSE :
			// сохраняем в column j-й столбец матрицы m2
			fld[m2 + 4 * edx + 48]
				fld[m2 + 4 * edx + 32]
				fld[m2 + 4 * edx + 16]
				fld[m2 + 4 * edx]
				fstp[column]
				fstp[column + 4]
				fstp[column + 8]
				fstp[column + 12]

				movups xmm0, [m1 + 4 * ebx]                       // копируем в xmm0 i-ю строку матрицы M1
				movups xmm1, [column]                             // копируем в xmm1 j-й столбец
				mulps  xmm0, xmm1                                 // перемножаем i-ю строку с j-м столбцом поэлементно
				movups[column], xmm0                             // переносим полученный вектор в column

				// сохраняем сумму всех элементов вектора
				fld[column]
				fld[column + 4]
				fld[column + 8]
				fld[column + 12]
				faddp  st(1), st(0)
				faddp  st(1), st(0)
				faddp  st(1), st(0)
				fstp[resultSSE2 + 4 * eax]                     // сохраняем сумму в k-й элемент resultSSE2

				// повторяем всё это для каждого элемента строки
				inc    eax
				inc    edx
				cmp    edx, N
				jne    GETTING_ELEMENT_SSE

				// повторяем всё это для каждой строки
				add    ebx, 4
				loop   COMPOSITING_SSE

				// получение матрицы result3
				mov    ecx, N* N
				mov    eax, 0

				// копия матрицы result2 в result3
				COPY_SSE:
				movups xmm0, [resultSSE2]
				movups xmm1, [resultSSE2 + 16]
				movups xmm2, [resultSSE2 + 32]
				movups xmm3, [resultSSE2 + 48]
				movups[resultSSE3 + 16], xmm1
				movups[resultSSE3 + 32], xmm2
				movups[resultSSE3 + 48], xmm3

				// складывание 1-й и 2-й строки и сохранение этой строки в первую строку
				ADDING_1_AND_2_SSE :
				addps  xmm0, xmm1
				movups[resultSSE3], xmm0
		}
		end = __rdtsc();

		// результаты теста SSE
		std::cout << std::endl << std::endl;
		printResults(true, resultSSE1, resultSSE2, resultSSE3, end - start);
	}

	return 0;
}