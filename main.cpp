#include <iostream>
#include <set>
#include <unordered_set>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <stack>
#include <iomanip>
#include <conio.h>

using namespace std;
class production {
private:
    string left;
    set<string> right;
    set<string>::iterator it;
    set<int> relateNT;

public:
    production () = default;
    production (string & pLeft, set<string> & pRight) {
        this->left = pLeft;
        this->right = pRight;
    }
    const string &getLeft() const {
        return left;
    }

    void setLeft(const string &l) {
        this->left = l;
    }

    const set<string> &getRight() const {
        return right;
    }

    void setRight(const set<string> &r) {
        this->right = r;
    }

    const set<string>::iterator &getIt() const {
        return it;
    }

    void setIt(const set<string>::iterator &i) {
        production::it = i;
    }

    const set<int> &getRelateNt() const {
        return relateNT;
    }

    void setRelateNt(const set<int> &reNt) {
        this->relateNT = reNt;
    }

    void print () {
        cout << this->left << "->";
        int cnt = 0;
        for (const auto& i : this->right) {
            cout << i;
            cnt++;
            if (cnt != this->right.size()) cout << '|';
        }
        cout << endl;
    }

    void insertRight (const string& str) {
        this->right.insert(str);
    }

    void eraseRight (const string& str) {
        this->right.erase(str);
    }

    void insertReNT (const int& n) {
        this->relateNT.insert(n);
    }

    void eraseReNT (const int& n) {
        this->relateNT.erase(n);
    }
};

vector<production> v;
vector<string> nonTerminal;
unordered_set<char> terminal;
stack<char> terStack;

unordered_map<string,set<char>> first;
unordered_map<string,set<char>> follow;
unordered_map<string,set<char>> Select;
bool isLL1 = true;
stack<string> anaStack;
stack<char> restStack;
int perFFGripWidth = 25;
int perGripWidth = 15;
const char finishChar = '#'; //当你的文法或句子中含有此符号时，请修改它为其他符号
const char divChar = '\''; //当你的文法或句子中含有此符号时，请修改它为其他符号

production mySplit(const string& str) {
    string vLeft;
    set<string> vRight;
    string temp;
    bool isSubFirst = true;
    bool isGTFirst = true;
    for (char i : str) {
        if (isSubFirst && i == '-') {
            isSubFirst = false;
            vLeft = temp;
            temp = "";
        } else if (isGTFirst && i == '>') {
            isGTFirst = false;
            continue;
        } else if (i == '|') {
            vRight.insert(temp);
            temp = "";
        } else {
            temp += i;
        }
    }
    if (!temp.empty()) vRight.insert(temp);
    return {vLeft,vRight};
}

void getProductions() {
    cout << "若一个非终结符可推出多个结果，请直接以“|”分隔，不必分开输入" << endl;
    cout << "输入产生式,以“" << finishChar << "”为结束标志:" << endl;
    string s;
    while (cin >> s && s[0] != finishChar)
        v.push_back(mySplit(s));
}

void getNonTerminal() {
    for (auto & i : v)
        nonTerminal.push_back(i.getLeft());
}

bool isNT(const string& s) {
    return find(nonTerminal.begin(), nonTerminal.end(), s) != nonTerminal.end();
}

void getTerminal() {
    terminal.insert(finishChar);
    for (int i=0; i<v.size(); ++i) {
        auto vi = v.begin() + i;
        set<string> vr = vi->getRight();
        for (auto j: vr) {
            if (j == "~") continue;
            string::iterator k = j.begin();
            while (k < j.end()) {
                string s = {*k};
                if (*(k+1) == divChar) {
                    s.append({divChar});
                    ++k;
                }
                ++k;
                if (!isNT(s)) terStack.push(s[0]);
            }
        }
    }
    while (!terStack.empty()) {
        terminal.insert(terminal.find(finishChar), terStack.top());
        terStack.pop();
    }
}

template <typename T>
void printVS(T& S) {
    int cnt = 1;
    int size = (int)S.size();
    if (size == 0) cout << "{}";
    else {
        cout << "{'";
        for (auto & i : S) {
            cout << "\033[34m" << i << "\033[0m";
            if (cnt != size) cout << "','";
            else cout << "'}";
            ++cnt;
        }
    }
}

bool isDirect(vector<production>::iterator i) {
    for (auto j = i->getRight().begin(); j != i->getRight().end(); ++j) {
        if (j->find(i->getLeft()) == 0) {
            i->setIt(j);
            return true;
        }
    }
    return false;
}

void simplify() {
    for (int i = 0; i < v.size(); ++i) {
        auto vi = v.begin() + i;
        string vl = vi->getLeft();
        set<string> vr =  vi->getRight();
        if (!isDirect(vi) && i != v.size()-1) {
            for (auto j = vr.begin(); j != vr.end();) {
                string str = *j;
                string s;
                bool hasSimplify = false;
                for (string::iterator k = str.begin(); k < str.end();) {
                    s = {*k};
                    if (k != str.end()-1 && *(k+1) == divChar) s.append({divChar});
                    if (isNT(s)) {
                        int l = i + 1;
                        for (;l < nonTerminal.size(); ++l) {
                            if (s == nonTerminal[l]) {
                                hasSimplify = true;
                                string afterReplace;
                                vi->insertReNT(l);
                                vi->eraseRight(str);
//                                cout << s << endl;
                                for (auto & m : v[l].getRight()) {
//                                    cout << m << endl;
                                    afterReplace = str;
                                    if (m != "~") {
                                        vi->insertRight(afterReplace.replace(k-str.begin(), s.length(), m));
                                    } else {
                                        afterReplace = afterReplace.replace(k-str.begin(), s.length(), "");
                                        if (afterReplace.empty()) vi->insertRight("~");
                                        else vi->insertRight(afterReplace);
                                    }
//                                    vi->print();
                                }
                                vr = vi->getRight();
                                j = vr.begin();
                                str = *j;
                                k = str.begin();
//                                cout << endl;
                                break;
                            }
                        }
                        if (l == nonTerminal.size()) hasSimplify = false;
                        if (!hasSimplify) {
                            ++k;
                            if (k != str.end()-1 && *(k+1) == divChar) ++k;
                        }
                    } else {
                        ++k;
                        if (k != str.end()-1 && *(k+1) == divChar) ++k;
                    }
                }
                if (hasSimplify) {
                    vr = vi->getRight();
                    j = vr.begin();
                } else ++j;
            }
            if (!vi->getRelateNt().empty()) {
                set<int> RN = vi->getRelateNt();
                for (auto & n : RN) {
                    v.erase(v.begin()+n);
                    nonTerminal.erase(nonTerminal.begin()+n);
                    vi->eraseReNT(n);
                }
            }
        }
    }
}

void removeRecursion() {
    for (int i = 0; i < v.size(); ++i) {
        auto vi = v.begin() + i;
        string vl = vi->getLeft();
        set<string> vr =  vi->getRight();
        if (isDirect(vi)) {
            string vii = *vi->getIt();
            for (const auto & j : vr) {
                if (j != vii) {
                    vi->insertRight(j + vl + divChar);
                }
                vi->eraseRight(j);
            }

            production p;
            p.setLeft(vl + divChar);
            p.insertRight(vii.erase(0,1) + vl + divChar);
            p.insertRight("~");
            v.emplace(vi+1, p);
            nonTerminal.emplace(nonTerminal.begin()+i+1, p.getLeft());

            ++i;
        }
    }
}

void generateFirstSet() {
    int cycleIndex = 1;
    bool isUpdate = false;

    for (const auto& nt : nonTerminal) first[nt];
    do {
        if (isUpdate) isUpdate = false;
        cout << "第\033[32m" << cycleIndex++ << "\033[0m次循环：" << endl;
        for (int i = 0; i< v.size(); ++i) {
            auto vi = v.begin() + i;
            string vl = vi->getLeft();
            set<string> vr = vi->getRight();
            string pro;
            for (auto j : vr) {
                pro = vl;
                pro += "->" + j;

                if (j == "~") {
                    if (first[vl].insert('~').second && !isUpdate)
                        isUpdate = true;
                } else {
                    set<char> pool;

                    for (string::iterator k = j.begin(); k < j.end();) {
                        string s = {*k};
                        int stepLength = 1;
                        if (k != j.end()-1 && *(k+1) == divChar) {
                            s.append({divChar});
                            ++stepLength;
                        }

                        if (isNT(s) && first[s].count('~')) {
                            for (auto l : first[s])
                                pool.insert(l);
                            k += stepLength;
                        } else {
                            if (isNT(s)) {
                                if (first[s].empty()) break;
                                for (auto l : first[s])
                                    pool.insert(l);
                            } else pool.insert(s[0]);
                            pool.erase('~');
                            break;
                        }
                    }

                    if (!pool.empty())
                        for (auto l :pool) {
                            if (l != '~') Select[pro].insert(l);
                            if (first[vl].insert(l).second && !isUpdate)
                                isUpdate = true;
                        }
                }

                cout << "对应产生式：        \033[32m" << pro << "\033[0m" <<endl;
                for (const auto& nt : nonTerminal) {
                    cout << "FIRST(\033[32m" << nt << "\033[0m) = ";
                    printVS(first[nt]);
                    cout << endl;
                }
            }
        }
        if (!isUpdate) cout << "本次循环无变化，退出" << endl;
        cout << endl;
    } while (isUpdate);
}

void generateFollowSet() {
    int cycleIndex = 1;
    bool isUpdate = false;

    for (const auto& nt : nonTerminal) follow[nt];
    follow[nonTerminal[0]].insert(finishChar);
    do {
        if(isUpdate) isUpdate = false;
        cout << "第\033[32m" << cycleIndex++ << "\033[0m次循环：" << endl;
        for (int i = 0; i < v.size(); ++i) {
            auto vi = v.begin() + i;
            string vl = vi->getLeft();
            set<string> vr =  vi->getRight();
            string pro;

            for (auto j : vr) {
                pro = vl;
                pro += "->" + j;

                if(j == "~") {
                    if (!follow[vl].empty())
                        for (auto fl : follow[vl])
                            Select[pro].insert(fl);
                    continue;
                }

                set<char> pool;
                bool haveBlank = true; //检查是否是候选式中最后一个字符是否是非终结符

                for (string::iterator k = j.end()-1; k >= j.begin();) {
                    string s = {*k};
                    int stepLength = 1;
                    if (*k == divChar) {
                        s.insert(0, {*(k-1)});
                        ++stepLength;
                    }

					if (isNT(s)) {
                        if (haveBlank) {
                            for (auto l : follow[vl])
                                if (follow[s].insert(l).second && !isUpdate)
                                    isUpdate = true;
                        }

                        if (!pool.empty())
                            for (auto l : pool)
                                if (follow[s].insert(l).second && !isUpdate)
                                    isUpdate = true;

                        if (!first[s].count('~')) {
                            pool.clear();
                            haveBlank = false;
                        }

                        for (auto l : first[s])
                            if (l != '~') pool.insert(l);
                    } else {
                        pool.clear();
                        pool.insert(s[0]);
                        haveBlank = false;
                    }

                    k -= stepLength;
                }

                cout << "对应产生式：        \033[32m" << vl << "->" << j << "\033[0m" <<endl;
                for (const auto& nt : nonTerminal) {
                    cout << "FOLLOW(\033[32m" << nt << "\033[0m) = ";
                    printVS(follow[nt]);
                    cout << endl;
                }
            }
        }
        if (!isUpdate) cout << "本次循环无变化，退出" << endl;
        cout << endl;
    }while (isUpdate);
}

void generateFirstAndFollowTable() {
    for (const auto& i : nonTerminal) {
        int maxLength = (int)max(first[i].size(), follow[i].size());
        maxLength = 4*maxLength+1;
        if (maxLength > perFFGripWidth) perFFGripWidth = maxLength + 10;
    }

    cout << " --------";
    for (int x = 0; x < 2; ++x)
    	cout << " " << string(perFFGripWidth, '-');
    cout << endl;

    int gap = (perFFGripWidth-5)/2;
    cout << "|非终结符|" << setw(gap+5) << "FIRST" << string(gap, ' ') << "|" << setw(gap+5) << "FOLLOW" << string(gap, ' ') << "|" << endl;

    for (const auto& i : nonTerminal) {
        int firstLength = (perFFGripWidth-4*(int)first[i].size()-1)/2;
        int followLength = (perFFGripWidth-4*(int)follow[i].size()-1)/2;
        cout << "|   " << left << setw(5) << i << "|" << string(firstLength, ' ');
        printVS(first[i]);
        cout << string(firstLength, ' ') << "|" << string(followLength, ' ');
        printVS(follow[i]);
        cout << string(followLength, ' ') << "|" << endl;
    }

    cout << " --------";
    for (int x = 0; x < 2; ++x)
        cout << " " << string(perFFGripWidth, '-');
}

void printSelectSetAndJudge() {
    for (int i = 0; i < v.size(); ++i) {
        auto vi = v.begin() + i;
        string vl = vi->getLeft();
        set<string> vr = vi->getRight();

        string pro;
        int size = 0;
        set<char> unions;

        for (const auto& j: vr) {
            pro = vl;
            pro += "->" + j;

            if (vr.size() > 1) {
                for (auto sc : Select[pro]) {
                    ++size;
                    unions.insert(sc);
                }
            }

            cout << "SELECT(\033[32m" << pro << "\033[0m) = ";
            printVS(Select[pro]);
            cout << endl;
        }

        if (vr.size() > 1) {
            if (size != unions.size()) {
                isLL1 = false;
                cout << "\033[31m有交集\033[0m" << endl << endl;
            } else cout << "\033[32m无交集\033[0m" << endl << endl;
        } else cout << endl;
    }
}

void generateParsingTable(vector<vector<string>>::iterator &parsingTable) {
    int col = 1;
    for (auto & ch : terminal)
        parsingTable[0][col++] = ch;
    int row = 1;
    for (auto & ch : nonTerminal)
        parsingTable[row++][0] = ch;

    for (int i = 1; i <= nonTerminal.size(); ++i) {
        for (int j = 1;j <= terminal.size(); ++j) {
            for (int k = 0; k < v.size(); ++k) {
                auto vk = v.begin() + k;
                set<string> vr = vk->getRight();
                for (const auto& l : vr) {
                    if (parsingTable[i][0] == vk->getLeft() && Select[parsingTable[i][0] + "->" + l].count(parsingTable[0][j][0])) {
                        parsingTable[i][j] = l;
                    }
                }
            }
        }
    }

    for (int i = 0; i <= terminal.size(); i++)
        cout << " " << string(perGripWidth, '-');
    cout << endl;

    int titleGap = (int)terminal.size()*(perGripWidth+1)-7;
    if (titleGap % 2)
        cout << "|   非终结符    |" << string(((int)terminal.size()*(perGripWidth+1)-7)/2, ' ') <<
                "终结符" << string(((int)terminal.size()*(perGripWidth+1)-7)/2+1, ' ') << "|" << endl;
    else
        cout << "|   非终结符    |" << string(((int)terminal.size()*(perGripWidth+1)-7)/2, ' ') <<
             "终结符" << string(((int)terminal.size()*(perGripWidth+1)-7)/2, ' ') << "|" << endl;
    int gripGap;
    for (int i = 0; i <= nonTerminal.size(); i++) {
        for (int j = 0;j <= terminal.size(); j++) {
            string str = parsingTable[i][j];
            if (j == 0) cout << "|";
            if (str.empty()) cout << string(perGripWidth, ' ') << "|";
            else {
                gripGap = perGripWidth-(int)str.length();
                if (gripGap % 2) {
                    if (i > 0 && j > 0) cout << string(gripGap/2+1, ' ') << "\033[34m" << str << "\033[0m" << string(gripGap/2, ' ') << "|";
                    else cout << string(gripGap/2+1, ' ') << str << string(gripGap/2, ' ') << "|";
                } else {
                    if (i > 0 && j > 0) cout << string(gripGap/2, ' ') << "\033[34m" << str << "\033[0m" << string(gripGap/2, ' ') << "|";
                    else cout << string(gripGap/2, ' ') << str << string(gripGap/2, ' ') << "|";
                }
            }
        }
        cout << endl;
    }
    for (int i = 0; i <= terminal.size(); i++)
        cout << " " << string(perGripWidth, '-');
    cout << endl;
}

void analyse(string s, vector<vector<string>> parsingTable) {
    anaStack.emplace(1, finishChar);
    anaStack.push(nonTerminal[0]);
    restStack.push(finishChar);
    reverse(s.begin(), s.end());
    for (char i : s) restStack.push(i);

    cout << " ----";
    for (int x = 0; x < 3; ++x)
        cout << " " << string(perGripWidth, '-');
    cout << endl;
    cout << "|步骤|" << left << setw(perGripWidth) << "分析栈" << "|"
                    << right<< setw(perGripWidth) << "剩余串" << "|"
                    << left << setw(perGripWidth) << "匹配结果" << "|" <<endl;

    bool isSuccess = true;
    int step = 1;
    do {
        printf("|\033[34m%4d\033[0m|", step++);

        stack<string> anaTemp = anaStack;
        string anaStr;
        bool isAnaFirst = true;
        while (!anaTemp.empty()) {
            if (isAnaFirst) {
                isAnaFirst = false;
                anaTemp.pop();
                continue;
            }
            anaStr.insert(0, anaTemp.top());
            anaTemp.pop();
        }
        cout << anaStr;

        int anaLength = int(anaStr.length() + anaStack.top().length());

        stack<char> restTemp = restStack;

        if (anaStack.top() == string(1, restStack.top())){
            string blank = string(2*perGripWidth+1-anaLength-restStack.size(), ' ');
            blank[perGripWidth - anaLength] = '|';
            printf("\033[32m%s\033[0m%s\033[32m%c\033[0m", anaStack.top().c_str(), blank.c_str(), restStack.top());

            bool isRestFirst = true;
            while (!restTemp.empty()) {
                if (isRestFirst) {
                    isRestFirst = false;
                    restTemp.pop();
                    continue;
                }
                printf("%c", restTemp.top());
                restTemp.pop();
            }

            string matchSuccess = anaStack.top() + "匹配";
            matchSuccess = matchSuccess.append(string(perGripWidth - matchSuccess.length(), ' '));
            printf("|\033[32m%s\033[0m|\n", matchSuccess.c_str());

            anaStack.pop();
            restStack.pop();
        } else {
            int i;
            int j;
            bool isMatch = false;
            for (i = 1; i <= nonTerminal.size(); i++) {
                if (isMatch) break;
                for (j = 1; j <= terminal.size(); j++) {
                    if (anaStack.top() == parsingTable[i][0] && restStack.top() == parsingTable[0][j][0]) {
                        string str = parsingTable[i][j];
                        if (!str.empty()) {
                            isMatch = true;

                            string blank = string(2*perGripWidth+1-anaLength-restStack.size(), ' ');
                            blank[perGripWidth - anaLength] = '|';
                            printf("\033[32m%s\033[0m%s\033[32m%c\033[0m", anaStack.top().c_str(), blank.c_str(), restStack.top());

                            bool isRestFirst = true;
                            while (!restTemp.empty()) {
                                if (isRestFirst) {
                                    isRestFirst = false;
                                    restTemp.pop();
                                    continue;
                                }
                                printf("%c", restTemp.top());
                                restTemp.pop();
                            }

                            string pro = anaStack.top() + "->" + str;
                            pro = pro.append(string(perGripWidth - pro.length(), ' '));
                            printf("|\033[32m%s\033[0m|\n", pro.c_str());

                            if (str == "~") {
                                anaStack.pop();
                            } else {
                                anaStack.pop();
                                for (string::iterator k = str.end()-1; k >= str.begin(); --k) {
                                    if (*k == '\'') {
                                        anaStack.push({*(k-1), '\''});
                                        --k;
                                    } else {
                                        anaStack.push({*k});
                                    }
                                }
                            }
                        }
                        break;
                    }
                }
            }
            if (!isMatch) {
                isSuccess = false;

                string blank = string(2*perGripWidth+1-anaLength-restStack.size(), ' ');
                blank[perGripWidth - anaLength] = '|';
                printf("\033[31m%s\033[0m%s\033[31m%c\033[0m", anaStack.top().c_str(), blank.c_str(), restStack.top());

                bool isRestFirst = true;
                while (!restTemp.empty()) {
                    if (isRestFirst) {
                        isRestFirst = false;
                        restTemp.pop();
                        continue;
                    }
                    printf("%c", restTemp.top());
                    restTemp.pop();
                }

                string matchFail = "匹配失败";
                matchFail = matchFail.append(string(perGripWidth - matchFail.length(), ' '));
                printf("|\033[31m%s\033[0m|\n", matchFail.c_str());
            }
        }

        if (!isSuccess) break;
    } while (!restStack.empty());
    cout << " ----";
    for (int x = 0; x < 3; ++x)
        cout << " " << string(perGripWidth, '-');
    cout << endl;
    if(restStack.empty()) printf("\033[32m匹配成功！\033[0m\n");
    else printf("\033[31m匹配失败！\033[0m\n");
}

int main() {
    getProductions();

    getNonTerminal();
    cout << endl << "非终结符集合为：" << endl;
    printVS(nonTerminal);
    cout << endl << endl;

    getTerminal();
    cout << "终结符集合为：" << endl;
    printVS(terminal);
    cout << endl << endl;

    cout << "是否进行化简处理?" << endl;
    cout << "[Y]是             [N]否" << endl;
    while (true) {
        if (_kbhit() && (_getch() == 78 || _getch() == 110))
            break;
        if (_kbhit() && (_getch() == 89 || _getch() == 121)) {
            simplify();
            cout << endl;
            for (auto & i : v) i.print();
            cout << endl;
            break;
        }
    }

    cout << "是否进行消除左递归处理?" << endl;
    cout << "[Y]是             [N]否" << endl;
    while (true) {
        if (_kbhit() && (_getch() == 78 || _getch() == 110))
            break;
        if (_kbhit() && (_getch() == 89 || _getch() == 121)) {
            removeRecursion();
            cout << endl;
            for (auto & i : v) i.print();
            cout << endl;
            break;
        }
    }

    cout << "更新后的非终结符集合为：" << endl;
    printVS(nonTerminal);
    cout << endl << endl;

    cout << "该文法的First集合分析过程为：" << endl;
    generateFirstSet();
    cout << endl;

    cout << "该文法的Follow集合分析过程为：" << endl;
    generateFollowSet();
    cout << endl;

    cout << "该文法的First集合和Follow集合为：" << endl;
    generateFirstAndFollowTable();
    cout << endl << endl;

    cout << "该文法的Select集合为：" << endl;
    printSelectSetAndJudge();

    if (isLL1) {
        cout << "\033[32m以上产生式左边相同的Select集合交集为空，是LL(1)文法\033[0m" << endl;
        cout << endl;

        vector<vector<string>> parsingTable(nonTerminal.size()+1, vector<string>(terminal.size()+1, ""));
        cout << "该文法的预测分析表为：" << endl;
        auto paIt = parsingTable.begin();
        generateParsingTable(paIt);
        cout << endl;

        while (true) {
            cout << "请输入要分析的句子：";
            string ana;
            cin >> ana;
            if (ana.length() > perGripWidth) perGripWidth = (int)ana.length() + 5;
            analyse(ana, parsingTable);
        }
    } else cout << "\033[31m以上产生式左边相同的Select集合交集不为空，不是LL(1)文法\033[0m" << endl;
    return 0;
}
