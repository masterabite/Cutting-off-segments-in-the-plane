#include <iostream>
#include <locale>
#include <fstream>

using namespace std;

bool isEqual(long double d1, long double d2) {
    return (abs(d1 - d2) <= 1e-6);
}

struct point {
    long double x, y;
    point() {
        x = 0.0;
        y = x;
    }

    point(long double _x, long double _y) {
        x = _x;
        y = _y;
    }

    void print() {
        printf("(%lf,%lf) ", x, y);
    }

    void scan(ifstream& fin) {
        fin >> x >> y;
    }

    bool parallelUnidirected(point p) {
        if (p.x * x < -1e-5 || p.y * y < -1e-5) {
            return false;
        }

        if (isEqual(p.x, 0.0) || isEqual(x, 0.0)) {
            return isEqual(p.x, x);
        }

        if (isEqual(p.y, 0.0) || isEqual(y, 0.0)) {
            return isEqual(p.y, y);
        }

        return isEqual(x / p.x, y / p.y);
    }

    point operator+(const point& p) const {
        return point(x + p.x, y + p.y);
    }
    point operator-(const point& p) const {
        return point(x - p.x, y - p.y);
    }
    long double operator*(const point& p) const {
        return x * p.x + y * p.y;
    }
    point operator*(long double k) const {
        return point(x * k, y * k);
    }

    double length() {
        return sqrt(x * x + y * y);
    }
};

struct line_segment {
    bool visible;
    point p0, p1;
    long double ta, tb;

    line_segment() {
        visible = true;
        p1 = point(0, 0);
        p0 = point(0, 0);
        ta = 0.0;
        tb = 1.0;
    }
    line_segment(point _p0, point _p1) {
        visible = true;
        p1 = _p1;
        p0 = _p0;
        ta = 0.0;
        tb = 1.0;
    }

    bool havePoint(point p) {
        return ((p-p0).parallelUnidirected(p1-p));
    }

    void scan(ifstream& fin) {
        p0.scan(fin);
        p1.scan(fin);
    }

    void print() {
        p0.print();
        cout << ' ';
        p1.print();
        cout << '\n';
    }

    point get_point(long double t) {
        return (p0 + (p1 - p0) * t);
    }

    long double get_t(point p) {
        point v = p1 - p0;
        point pn = p - p0;
        if (pn.x/v.x == pn.y/v.y) {
            return pn.x / v.x;
        }
        else {
            return DBL_MAX;
        }
    }
};

struct square {
    point p[4];

    void scan(ifstream& fin) {
        p[0].scan(fin);
        p[2].scan(fin);

        p[1].x = p[2].x;
        p[1].y = p[0].y;

        p[3].x = p[0].x;
        p[3].y = p[2].y;
    }
};

long double getPointSpacing(point p1, point p2, point p3) {
    return (p1-p2).length() + (p3-p2).length();
}

long double get_tE(line_segment& l1, line_segment& l2) {
    long double mint = -10, maxt = 10;
    point minp(l1.get_point(mint)), maxp(l1.get_point(maxt));
    long double l = l2.ta, r = l2.tb;
    
    long double sl = getPointSpacing(minp, l2.get_point(l), maxp);
    long double sm{};
    long double sr = getPointSpacing(minp, l2.get_point(r), maxp);

    while (r - l > 1e-12) {
        long double m = (l + r) / 2;
        sm = getPointSpacing(minp, l2.get_point(m), maxp);

        if (sr - sl >= 1e-18) {
            sr = sm;
            r = m;
        }
        else {
            sl = sm;
            l = m;
        }
    }

    return r;
}

//сначала секущий отрезок
void cutOff(line_segment& l1, line_segment& l2) {

    point n = point(l1.p1.y - l1.p0.y, -(l1.p1.x-l1.p0.x)); //вектор нормали секущего отрезка, направленый вправо по направлению вектора
    long double tE = get_tE(l1, l2); //находим параметр точки пересечения для отсекаемого отрезка
    point interPoint = l2.get_point(tE);

    long double prod = (l2.p1 - l1.p0) * n;


    if (l1.havePoint(interPoint)) {
        if (prod < 0) {
            l2.p0 = interPoint;
            l2.ta = tE;
        }
        else {
            l2.p1 = interPoint;
            l2.tb = tE;
        }
    }
    else {
        point pf(l1.p1 - l1.p0), ps(l2.p0 - l1.p0);
        long double k = pf.x * ps.y - pf.y * ps.x;
        if (k < 0) {
            l2.visible = false;
        }
    }

}

int main() {

    setlocale(LC_ALL, "Russian");
    ifstream fin("in.txt");
    if (!fin.is_open()) {
        cout << "не удалось откыть файл!";
        return 0;
    }

    square s;
    s.scan(fin);
    int n; fin >> n;
    line_segment* a = new line_segment[n];
    for (int i = 0; i < n; ++i) {
        a[i].scan(fin);
    }fin.close();

    for (int q = 0; q < 4; ++q) {
        line_segment sline1(s.p[q], s.p[(q+1)%4]);

        for (int i = 0; i < n; ++i) {
            cutOff(sline1, a[i]);
        }
    }

    int count = 0;
    for (int i = 0; i < n; ++i) {
        if (a[i].visible) {
            ++count;
        }
    }

    ofstream fout("out.txt");
    fout << count << '\n';

    for (int i = 0; i < n; ++i) {
        if (a[i].visible) {
            fout << (a[i].p0.x < 1e-6? 0: a[i].p0.x) << ' ' << (a[i].p0.y < 1e-6 ? 0 : a[i].p0.y) << ' ';
            fout << (a[i].p1.x < 1e-6 ? 0 : a[i].p1.x) << ' ' << (a[i].p1.y < 1e-6 ? 0 : a[i].p1.y) << '\n';
        }
    } fout.close();
    return 0;
}
