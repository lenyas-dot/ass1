#include <iostream>
#include <intrin.h>
#include <random>
#include <ctime>
#define N 4
#define MAX_RANDOM 20

/*
* ������� 11
*
* ����������� ������ 4�4. ��������� ��� �������� ������� �1 � 2 ����� ���. �������� �1 �� ������� �2.
* ������� ����������� ������ � ������ ������ �������.
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

void printResults(bool isSSE, QuadraticMatrix& result1, QuadraticMatrix& result2, QuadraticMatrix& result3, __int64 ti�ks) {
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
	* � ���������� NO SSE ������������ ������� �����:
	* - � result1 ����� ��������� M1 / 2;
	* - � result2 ����� ��������� M1 * M2 (��������� ����� ��������� �� https://planetcalc.ru/1208);
	*/
	{
		start = __rdtsc();
		_asm {
			finit                                             // ������������� ������������

		// ��������� ������� result1
		mov    ecx, N* N
		mov    eax, 0                                     // ����� eax ����� �������� k-� ��������� ������

		REDUCING_NO_SSE:
			fld[m1 + 4 * eax]                             // ������ � ���� k-� ������� M1
				fld    m                                          // ������ � ���� m
				fdivp  st(1), st(0)                               // ����� k-� ������� m1 �� m
				fstp[result1 + 4 * eax]                        // ����������� ���������� ������� � k-� ������� result1
				inc    eax
				loop   REDUCING_NO_SSE

				// ��������� ������� result2
				mov    ecx, N
				xor eax, eax                                   // ����� eax ����� �������� �������� k � M1
				xor ebx, ebx                                   // ����� ebx ����� ������� i-� ������ M1

				COMPOSITING_NO_SSE :
			xor edx, edx                                   // ����� edx ����� ������� j-�� ������� M2

				GETTING_ELEMENT_NO_SSE :
			// �������� i-� ������ � j-� ������� �����������
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

				// ��������� ����� 4-� ��������� ��������� � �����
				faddp  st(1), st(0)
				faddp  st(1), st(0)
				faddp  st(1), st(0)
				fstp[result2 + 4 * eax]                        // ��������� ����� � k-� ������� result2

				// ��������� �� ��� ��� ������� �������� ������
				inc    eax
				inc    edx
				cmp    edx, N
				jne    GETTING_ELEMENT_NO_SSE

				// ��������� �� ��� ��� ������ ������
				add    ebx, 4
				loop   COMPOSITING_NO_SSE

				// ��������� ������� result3
				mov    ecx, N* N
				mov    eax, 0

				// ����� ������� result2 � result3
				COPY_NO_SSE:
			fld[result2 + 4 * eax]
				fstp[result3 + 4 * eax]
				inc    eax
				loop   COPY_NO_SSE

				mov    ecx, N
				mov    eax, 0

				ADDING_1_AND_2_NO_SSE:
			// ������ � ���� k-� �������� 1-� � 2-� ����� ������� result3
			fld[result3 + 4 * eax]
				fld[result3 + 4 * eax + 16]
				faddp  st(1), st(0)
				fstp[result3 + 4 * eax]
				inc    eax
				loop   ADDING_1_AND_2_NO_SSE
		}
		end = __rdtsc();

		// ���������� ����� NO SSE
		std::cout << std::endl << std::endl;
		printResults(false, result1, result2, result3, end - start);
	}

	/*
	* � ���������� SSE ������������ ������� �����:
	* - � resultSSE1 ����� ��������� M1 / m;
	* - � resultSSE2 ����� ��������� M1 * M2 (��������� ����� ��������� �� https://planetcalc.ru/1208).
	*/
	{
		start = __rdtsc();
		_asm {
			finit                                             // ������������� ������������

		// ��������� ������� �� N �������� ����� m
		mov    ecx, N
		xor eax, eax

		MAKING_VECTOR :
			fld    m
				fstp[vector + eax]
				add    eax, 4
				loop   MAKING_VECTOR

				// ��������� ������� resultSSE1
				mov    ecx, N
				xor eax, eax                                   // ����� eax ����� �������� 1-�� �������� � i-� ������ M1

				REDUCING_SSE :
				movups xmm0, [m1 + 4 * eax]                       // �������� � xmm0 i-� ������ ������� M1
				movups xmm1, [vector]                             // �������� � xmm1 ������, ��������� �� N �������� ����� m
				divps  xmm0, xmm1                                 // ��������� � m ��� i-� ������ M1
				movups[resultSSE1 + 4 * eax], xmm0               // ��������� ��� ����������� � m ��� ������ � resultSSE1
				add    eax, 4
				loop   REDUCING_SSE

				// ��������� ������� resultSSE2
				mov    ecx, N
				xor eax, eax                                   // ����� eax ����� �������� �������� k � M1
				xor ebx, ebx                                   // ����� ebx ����� �������� 1-�� �������� � i-� ������ M1

				COMPOSITING_SSE :
			xor edx, edx                                   // ����� edx ����� ������� j-�� ������� M2

				GETTING_ELEMENT_SSE :
			// ��������� � column j-� ������� ������� m2
			fld[m2 + 4 * edx + 48]
				fld[m2 + 4 * edx + 32]
				fld[m2 + 4 * edx + 16]
				fld[m2 + 4 * edx]
				fstp[column]
				fstp[column + 4]
				fstp[column + 8]
				fstp[column + 12]

				movups xmm0, [m1 + 4 * ebx]                       // �������� � xmm0 i-� ������ ������� M1
				movups xmm1, [column]                             // �������� � xmm1 j-� �������
				mulps  xmm0, xmm1                                 // ����������� i-� ������ � j-� �������� �����������
				movups[column], xmm0                             // ��������� ���������� ������ � column

				// ��������� ����� ���� ��������� �������
				fld[column]
				fld[column + 4]
				fld[column + 8]
				fld[column + 12]
				faddp  st(1), st(0)
				faddp  st(1), st(0)
				faddp  st(1), st(0)
				fstp[resultSSE2 + 4 * eax]                     // ��������� ����� � k-� ������� resultSSE2

				// ��������� �� ��� ��� ������� �������� ������
				inc    eax
				inc    edx
				cmp    edx, N
				jne    GETTING_ELEMENT_SSE

				// ��������� �� ��� ��� ������ ������
				add    ebx, 4
				loop   COMPOSITING_SSE

				// ��������� ������� result3
				mov    ecx, N* N
				mov    eax, 0

				// ����� ������� result2 � result3
				COPY_SSE:
				movups xmm0, [resultSSE2]
				movups xmm1, [resultSSE2 + 16]
				movups xmm2, [resultSSE2 + 32]
				movups xmm3, [resultSSE2 + 48]
				movups[resultSSE3 + 16], xmm1
				movups[resultSSE3 + 32], xmm2
				movups[resultSSE3 + 48], xmm3

				// ����������� 1-� � 2-� ������ � ���������� ���� ������ � ������ ������
				ADDING_1_AND_2_SSE :
				addps  xmm0, xmm1
				movups[resultSSE3], xmm0
		}
		end = __rdtsc();

		// ���������� ����� SSE
		std::cout << std::endl << std::endl;
		printResults(true, resultSSE1, resultSSE2, resultSSE3, end - start);
	}

	return 0;
}