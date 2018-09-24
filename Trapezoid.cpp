#include <stdio.h>

#define COUNTOF(array)	(sizeof(array)/sizeof((array)[0]))

struct Point {
	int x, y;
};

bool search_x_pos(int* result, int x, const Point* table, int num) {
	int i = 0;
	bool ret = true;

	// table[i].x <= x < table[i+1].x となる i を探す
	for (i = 0; i < num-1; ++i) {
		if (x < table[i+1].x) {
			break;
		}
	}

	// result の値
	// 引数 x が table[] の表す関数の範囲外のときは端のindexが入る。
	// (x              <  table[0].x) のとき i = 0
	// (table[num-1].x <= x         ) のとき i = num-1
	if (result) {
		// result が NULL でなけば代入
		*result = i;
	}

	// 引数 x が範囲外のときは false を返す。
	if ((x < table[0].x) || table[num-1].x <= x) {
		ret = false;
	}
	
	return ret;
}

// 写像(xからyへの変換)
// table[] が表す関数 y=f(x) に x を代入し y を得る。
//
// 関数 y=f(x) が通る点を table[] に指定する。
// 関数 y=f(x) を十分表現できるだけの点を指定すること。
// このとき、table[]が示す関数の点と点の間は内分比を使って補間する。
bool map_x2y(int* y, int x, const Point* table, int num) {
	bool ret = 0;
	int ratio = 0;								/* 内分比。 0x100を1と見なす固定小数点数として使う */
	int result = 0;
	int i = 0;

	// table[i].x <= x < table[i+1].x となる i を探す
	ret = search_x_pos(&i, x, table, num);

	// table[i].x と table[i+1].x の差に対する table[i].x と x の差の比率 ratio を計算
	{
		int step_x  = table[i+1].x - table[i].x;
		int delta_x =            x - table[i].x;
		ratio = (delta_x << 8) / step_x;
	}

	// table[i].y と table[i+1].y の差に ratio を掛け、
	// それを table[i].y に加算して補間位置を求める。
	{
		int step_y  = table[i+1].y - table[i].y;
		int delta_y = (step_y * ratio) >> 8;
		result = table[i].y + delta_y;
	}

	// NULL チェック
	if (y) {
		*y = result;
	}

	return ret;
}

// ↓いまいち。カーブ部分が直線になってしまうなあ。
// 
// 写像(xからyへの変換)
// table[] が表す関数 y=f(x) に x を代入し y を得る。
// map_x2y()と異なり、table[]が示す関数の点でカクッと急に変化せず
// なめらかに変化する。 smooth_coef を大きくするとなめらかさが増す。
//
// 関数 y=f(x) が通る点を table[] に指定する。
// 関数 y=f(x) を十分表現できるだけの点を指定すること。
// このとき、table[]が示す関数の点と点の間は内分比を使って補間する。
bool map_x2y_smooth(int* y, int x, const Point* table, int num, int smooth_coef) {
	int left = 0;
	int right = 0;
	bool ret_left  = map_x2y(&left,  x - smooth_coef, table, num);
	bool ret_right = map_x2y(&right, x + smooth_coef, table, num);

	if (!ret_left || !ret_right) {
		// 片方とれないときは smoothing できないのでしない。
		return map_x2y(y, x, table, num);
	}

	if (y) {
		*y = (left + right) >> 1;
	}

	return true;
}

int main(int argc, char* argv[])
{
	// 台形関数
	Point trapezoid[] = {
		{0x0000, 0x0000},
		{0x0010, 0x0100},
		{0x0070, 0x0100},
		{0x0080, 0x0000},
	};

	for (int x = 0; x < 0x90; ++x) {
		int y0, y1;
		map_x2y(&y0, x, trapezoid, COUNTOF(trapezoid));
		map_x2y_smooth(&y1, x, trapezoid, COUNTOF(trapezoid), 8);
		printf("%d, %d, %d\n", x, y0, y1);
	}

	return 0;
}
