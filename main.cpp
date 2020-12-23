#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <exception>
#include <stdexcept>

using namespace std;

class Point {
private:
    int x;
    int y;
public:
    Point(int xCoord, int yCoord) {
        x = xCoord;
        y = yCoord;
    }

    int X() { return x; }

    int Y() { return y; }
};

class Stack {
private:
    vector<Point> stack;
    int max;
public:
    const int MaxValue = 1000;

    Stack(int max) {
        if (max > MaxValue) {
            throw range_error("Ошибка. Размер превышает максимально допустимый.");
        }
        this->max = max;
    }

    int size() {
        return stack.size();
    }

    bool isEmpty() {
        return size() == 0;
    }

    Point nextToTop() {
        if (isEmpty()) {
            throw out_of_range("Ошибка. Стек пуст.");
        }
        if (size() < 2) {
            throw out_of_range("Ошибка. В стеке недостаточно элементов.");
        }
        return stack[size() - 2];
    }

    Point top() {
        if (isEmpty()) {
            throw out_of_range("Ошибка. Стек пуст.");
        }
        return stack[size() - 1];
    }

    Point pop() {
        if (isEmpty()) {
            throw out_of_range("Ошибка. Стек пуст.");
        }
        Point last = stack[size() - 1];
        stack.pop_back();
        return last;
    }

    void push(Point item) {
        if (size() == max) {
            throw range_error("Ошибка. Достигнуто максимально допустимое кол-во элементов в стеке.");
        }
        stack.push_back(item);
    }
};

const double PI = 3.141592653589793; // 15 знаков после точки

/// Считывает данные из файла
vector<struct Point> readFromFile(int &N, const string &path);

/// Парсит строку в две координаты и возвращает объект типа Point
Point parceString(const string &s);

/// Формирует строку формата wkt, содержащие изначально введенные координаты
string multiPointString(vector<Point> points);

/// Вывод текста в файл
void writeToFile(const string &path, const string &out);

/// Организует вывод строки в файл, добавляя ее к уже имеющемуся содержанию файла
void appendToFile(const string &path, const string &output);

/// Сортирует вектор сначала по Y, после по X
vector<Point> sortYX(vector<Point> points);

/// Сортирует точки в порядке возрастания полярного угла
vector<Point> sortPoints(vector<Point> points, Point q);

/// Алгоритм Грэхема
Stack graham(int N, Point q, vector<Point> points);

/// Позволяет работать с файлами как через полный, так и через относительный пути
string path(string path);

/// Высчитывает полярный угол относительно точки q
double angle(Point p, Point q);

/// Определяет, образуют ли три точки a, b, c левый поворот
static bool left(Point a, Point b, Point c);

/// Проверяет, существует ли файл
bool exist(const string &name);

/// Вывод результата в файл.
void out(const string &path, const string &format, const string &clockwise, const Stack &stack);

/// Формирует строку формата "plain"
string plain(Stack stack, const string &clock);

/// Формирует строку формата "wkt"
string wkt(Stack stack, const string &clock);

int main(int argc, char *argv[]) {
    // Количество деревьев
    int N = 0;
    string clockwise = argv[1];
    string format = argv[2];
    // Коллекция со всеми поступившими координатами
    vector<Point> points = readFromFile(N, path(argv[3]));
    if (format == "wkt") {
        //Вывести в файл текст в формате MultiPointString
        writeToFile(path(argv[4]), multiPointString(points));
    }
    points = sortYX(points);
    // Точка с минимальной координатой Y или, если таких несколько,
    // самая "левая" из них(т.е. с минимальным значением X)
    Point q = points[0];
    points.erase(points.begin());
    // Точки, отсортированные в порядке возрастания полярного угла,
    // измеряемого против часовой стрелки относительно точки q.
    points = sortPoints(points, q);
    Stack stack = graham(N, q, points);
    out(path(argv[4]), format, clockwise, stack);
    return 0;
}

string plain(Stack stack, const string &clock) {
    string res;
    res += to_string(stack.size()) + "\n";
    Point point = Point(0, 0);
    int size = stack.size();
    if (clock == "cw") {
        vector<Point> v;
        for (int i = 0; i < size; ++i) {
            v.push_back(stack.pop());
        }
        Point pointQ = v[v.size() - 1];
        res += to_string(pointQ.X()) + " " + to_string(pointQ.Y()) + "\n";
        for (int i = 0; i < v.size(); ++i) {
            if (i != v.size() - 1) {
                point = v[i];
                res += to_string(point.X()) + " " + to_string(point.Y()) + "\n";
            }
        }
    } else {
        vector<Point> v;
        for (int i = 0; i < size; ++i) {
            point = stack.pop();
            v.push_back(point);
        }
        for (int i = 0; i < size; ++i) {
            point = v[size - i - 1];
            res += to_string(point.X()) + " " + to_string(point.Y()) + "\n";
        }
    }
    return res;
}

string wkt(Stack stack, const string &clock) {
    string tempRes;
    string res;
    int size = stack.size();
    Point point = Point(0, 0);
    Point pointQ = Point(0, 0);
    if (clock == "cw") {
        vector<Point> v;
        for (int i = 0; i < size; ++i) {
            v.push_back(stack.pop());
        }
        pointQ = v[v.size() - 1];
        for (int i = 0; i < size; ++i) {
            if (i != v.size() - 1) {
                point = v[i];
                tempRes += to_string(point.X()) + " " + to_string(point.Y()) + ", ";
            }
        }
    } else {
        vector<Point> v;
        for (int i = 0; i < size; ++i) {
            v.push_back(stack.pop());
        }
        pointQ = v[v.size() - 1];
        for (int i = 0; i < size; ++i) {
            if (size - i - 1 != size - 1) {
                point = v[size - i - 1];
                tempRes += to_string(point.X()) + " " + to_string(point.Y()) + ", ";
            }
        }
    }
    res = "\nPOLYGON ((" + to_string(pointQ.X()) + " " + to_string(pointQ.Y()) + ", ";
    res += tempRes;
    res += to_string(pointQ.X()) + " " + to_string(pointQ.Y()) + "))";
    return res;
}

void out(const string &path, const string &format, const string &clockwise, const Stack &stack) {
    if (format == "plain") {
        writeToFile(path, plain(stack, clockwise));
    } else {
        appendToFile(path, wkt(stack, clockwise));
    }
}

void appendToFile(const string &path, const string &output) {

    std::ofstream outfile;
    // Добавляет текст, вместо переписывания файла
    outfile.open(path, std::ios_base::app);
    outfile << output;
}

bool exist(const string &name) {
    ifstream f(name.c_str());
    return f.good();
}

string path(string path) {
    if (exist(path))
        return path;
    if (exist("input/" + path))
        return "input/" + path;
    if (exist("output/" + path))
        return "output/" + path;
    throw invalid_argument("Ошибка. Такого файла не существует.");
}

static bool left(Point a, Point b, Point c) {
    return (b.X() - a.X()) * (c.Y() - b.Y()) - (b.Y() - a.Y()) * (c.X() - b.X()) > 0;
    // >0 - left
    // =0 - on the line
    // <0 - right
}

Stack graham(int N, Point q, vector<Point> points) {
    Stack stack = Stack(N);
    stack.push(q);
    stack.push(points[0]);
    for (int i = 1; i < points.size(); i++) {
        while (stack.size() >= 2 && !left(stack.nextToTop(), stack.top(), points[i])) {
            stack.pop();
        }
        stack.push(points[i]);
    }
    return stack;
}

double angle(Point p, Point q) {
    if (p.X() == q.X() && p.Y() == q.Y()) return 0;
    if (p.X() - q.X() == 0) return PI / 2;
    if (p.X() - q.X() < 0)
        return atan((p.Y() - q.Y()) / (double) (p.X() - q.X())) + PI;
    return atan((p.Y() - q.Y()) / (double) (p.X() - q.X()));
}

vector<Point> sortPoints(vector<Point> points, Point q) {
    Point temp = Point(0, 0);
    for (int i = 0; i < points.size() - 1; i++) {
        for (int j = 0; j < points.size() - i - 1; j++) {
            if (angle(points[j], q) > angle(points[j + 1], q)) {
                // меняем элементы местами
                temp = points[j];
                points[j] = points[j + 1];
                points[j + 1] = temp;
            }
        }
    }
    return points;
}

vector<Point> sortYX(vector<Point> points) {
    Point temp = Point(0, 0);
    for (int i = 0; i < points.size() - 1; i++) {
        for (int j = 0; j < points.size() - i - 1; j++) {
            if (points[j].Y() > points[j + 1].Y()) {
                // меняем элементы местами
                temp = points[j];
                points[j] = points[j + 1];
                points[j + 1] = temp;
            }
        }
    }
    for (int i = 0; i < points.size() - 1; i++) {
        for (int j = 0; j < points.size() - i - 1; j++) {
            if (points[j].Y() == points[j + 1].Y() && points[j].X() > points[j + 1].X()) {
                // меняем элементы местами
                temp = points[j];
                points[j] = points[j + 1];
                points[j + 1] = temp;
            }
        }
    }
    return points;
}

vector<Point> readFromFile(int &N, const string &path) {
    string line;
    string nString;
    vector<Point> v;
    // Открываем файл для чтения
    std::ifstream in(path);
    if (in.is_open()) {
        getline(in, nString);
        while (getline(in, line)) {
            v.push_back(parceString((line)));
        }
    }
    // Закрываем файл
    in.close();
    N = atoi(nString.c_str());
    return v;
}

Point parceString(const string &s) {
    std::istringstream iss(s);
    std::vector<std::string> results(std::istream_iterator<std::string>{iss},
                                     std::istream_iterator<std::string>());
    Point point = Point(atoi(results[0].c_str()), atoi(results[1].c_str()));
    return point;
}

string multiPointString(vector<Point> points) {
    string res = "MULTIPOINT (";
    for (int i = 0; i < points.size(); i++) {
        res += "(" + to_string(points[i].X()) + " " + to_string(points[i].Y()) + ")";
        if (i < points.size() - 1) {
            res += ", ";
        }
    }
    res += ")";
    return res;
}

void writeToFile(const string &path, const string &output) {
    std::ofstream out; // поток для записи
    out.open(path); // окрываем файл для записи
    if (out.is_open()) {
        out << output;
    }
}
