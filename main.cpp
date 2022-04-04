#include <iostream>
#include <locale>
#include <fstream>
#include <cmath>
#include <cfloat>

using namespace std;

bool isEqual(long double d1, long double d2) {
    return (abs(d1 - d2) <= 1e-10);
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

    void print() const {
        cout << "(" << x << "; " << y << ")";
    }

    void scan(ifstream& fin) {
        fin >> x >> y;
    }

    bool parallelUnidirected(point p) const {
        if (p.x * x < -1e-8 || p.y * y < -1e-8) {
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
        return {x + p.x, y + p.y};
    }
    point operator-(const point& p) const {
        return {x - p.x, y - p.y};
    }
    long double operator*(const point& p) const {
        return x * p.x + y * p.y;
    }

    point operator*(long double k) const {
        return {x * k, y * k};
    }

    double length() const {
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

    bool havePoint(point p) const {
        return ((p - p0).parallelUnidirected(p1 - p));
    }

    void scan(ifstream& fin) {
        p0.scan(fin);
        p1.scan(fin);
    }

    void print() const {
        p0.print();
        cout << ' ';
        p1.print();
        cout << '\n';
    }

    point get_point(long double t) const {
        return (p0 + (p1 - p0) * t);
    }

    long double get_t(point p) const {
        point v = p1 - p0;
        point pn = p - p0;
        if (pn.x / v.x == pn.y / v.y) {
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

long double get_tE(line_segment& l1, line_segment& l2) {

    long double l = l2.ta, r = l2.tb;
    bool hor = (l1.p0.y == l1.p1.y);
    long double e = (hor ? l1.p0.y : l1.p0.x);


    while (r - l > 1e-12) {
        long double m = (l + r) / 2;

        point lp = l2.get_point(l);
        point mp = l2.get_point(m);
        point rp = l2.get_point(r);
        long double del = e - (hor ? lp.y : lp.x);
        long double dem = e - (hor ? mp.y : mp.x);
        long double der = e - (hor ? rp.y : rp.x);

        if (del * dem < 0) {
            r = m;
        }
        else if (der * dem < 0) {
            l = m;
        }
        else {
            if (dem == 0 && (del != 0)) {
                return m;
            }

            if (hor) {
                if (abs(lp.y - e) < abs(rp.y - e)) {
                    return l;
                }
                else {
                    return r;
                }
            }
            else {
                if (abs(lp.x - e) < abs(rp.x-e)) {
                    return l;
                }
                else {
                    return r;
                }
            }
        }
    }
    return r;
}

//сначала секущий отрезок
void cutOff_CyrusBeck(square& s, line_segment& l2) {
    for (int q = 0; q < 4; ++q) {
        line_segment l1(s.p[q], s.p[(q + 1) % 4]); //определяем отрезок стороны прямоугольника

        //вектор нормали секущего отрезка, направленый вправо по направлению вектора
        point n = point(l1.p1.y - l1.p0.y, -(l1.p1.x - l1.p0.x));
        long double prod = (l2.p1 - l1.p0) * n;

        //находим параметр точки пересечения для отсекаемого отрезка
        long double tE = get_tE(l1, l2);
        point interPoint = l2.get_point(tE);

        if (l1.havePoint(interPoint)) {
            if (prod < 0) {
                l2.p0 = interPoint;
            } else {
                l2.p1 = interPoint;
            }
        } else {
            point pf(l1.p1 - l1.p0), ps(l2.p0 - l1.p0);
            long double k = pf.x * ps.y - pf.y * ps.x;
            if (k < 0) {
                l2.visible = false;
            }
        }
    }
}


int getCode(square& s, point& p) {
    int pw[4] = {1, 2, 4, 8};
    int ans = 0;

    if (p.x < s.p[0].x) {
        ans += pw[0];
    }
    if (p.x > s.p[2].x) {
        ans += pw[1];
    }
    if (p.y < s.p[0].y) {
        ans += pw[2];
    }
    if (p.y > s.p[2].y) {
        ans += pw[3];
    }
    return ans;
}

void cutOff_CohenSutherland(square& s, line_segment& l2) {
    int code1 = getCode(s, l2.p0);
    int code2 = getCode(s, l2.p1);

    bool used = false; //флаг, который обозначает что прямая l2 пересекает прямоугольник s

    if (code1+code2 == 0) {
        return;
    }

    if ((code1 & code2) == 0) {
        for (int q = 0; q < 4; ++q) {
            line_segment l1(s.p[q], s.p[(q + 1) % 4]); //определяем отрезок стороны прямоугольника
            long double tE = get_tE(l1, l2);
            point interPoint = l2.get_point(tE);

            if (l1.havePoint(interPoint)) {
                used = true;
                //заменяем нужную точку\
                для каждого отрезка прямоугольника свое условие
                if ((q == 0 && l2.p0.y < l2.p1.y) ||
                    (q == 1 && l2.p0.x > l2.p1.x) ||
                    (q == 2 && l2.p0.y > l2.p1.y) ||
                    (q == 3 && l2.p0.x < l2.p1.x)) {

                    l2.p0 = interPoint;
                } else {
                    l2.p1 = interPoint;
                }
            }
        }
    } else {
        l2.visible = false;
    }

    //если отрезок так и не пересекся с прямоугольником, убираем его
    if (!used) {
        l2.visible = false;
    }
}

bool clipping(const string& fin_name,
                              const string& fout_name,
                              void (*cutOff)(square&, line_segment&)) {
    ifstream fin(fin_name);
    if (!fin.is_open()) {
        cout << "не удалось откыть файл!";
        return false;
    }

    square s;
    s.scan(fin);
    int n; fin >> n;
    auto* a = new line_segment[n];
    for (int i = 0; i < n; ++i) {
        a[i].scan(fin);
    }fin.close();


    for (int i = 0; i < n; ++i) {
        cutOff(s, a[i]);
    }

    int count = 0;
    for (int i = 0; i < n; ++i) {
        if (a[i].visible) {
            ++count;
        }
    }

    ofstream fout(fout_name);
    fout << count << '\n';

    for (int i = 0; i < n; ++i) {
        if (a[i].visible) {
            fout.precision(10);
            fout << fixed;
            fout << a[i].p0.x << ' ' << a[i].p0.y << ' ';
            fout << a[i].p1.x << ' ' << a[i].p1.y << '\n';
        }
    } fout.close();
    return true;
}

int main() {
    setlocale(LC_ALL, "Russian");
    clipping("in.txt", "out_CyrusBeck.txt", cutOff_CyrusBeck);
    clipping("in.txt", "out_CohenSutherland.txt", cutOff_CohenSutherland);
}
