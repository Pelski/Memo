#include <iostream>
#include <ctime>
#include <string>
#include <list>
#include <algorithm>
#include "colors.h"

using namespace std;

#define TASKS_FILE "tasks.dat"

enum Command {
    T_NONE,
    T_ADD,
    T_LAST,
    T_LIST,
    T_RM,
    T_PENDING,
    T_DONE,
    T_FIND,
    T_EDIT,
    T_I,
    T_II,
    T_III,
    T_IIII,
    T_FIVE,
    T_ACTUAL
};

enum TaskPriority {
    NORMAL,
    HIGH,
    HIGHER,
    HIGHEST
};

enum TaskStatus {
    NONE,
    PENDING,
    DONE,
    ALL
};

typedef struct Task {
    int ID;
    string taskname;
    string date;
    int status;
    int priority;
    string project;
    string tags;

    bool operator()(Task const& m) const {
        return m.ID == ID;
    }
} _task;

typedef struct String {
    string text;
} _string;

list<Task> tasks;
Task taskNew;

void showError(string err) {
    cerr << termcolor::bold << termcolor::red << " >> " << err << termcolor::reset << endl;
}

bool startsWithCaseInsensitive(string mainStr, string toMatch) {
    transform(mainStr.begin(), mainStr.end(), mainStr.begin(), ::tolower);
    transform(toMatch.begin(), toMatch.end(), toMatch.begin(), ::tolower);

    return mainStr.find(toMatch) == 0;
}

bool compareStrings(string s1, string s2) {
    return s1 == s2;
}

void displayTask(Task task) {
    string tmp = "#" + to_string(task.ID);
    cout << termcolor::cyan;
    cout.width(7);
    cout << std::right << tmp << " ";

    cout.width(15);
    cout << termcolor::green << task.date << " -- ";
    cout << termcolor::bold;

    switch (task.status) {
        case NONE: cout << "[ ]"; break;
        case PENDING: cout << "[•]"; break;
        case DONE: cout << "[√]"; break;
    }

    cout << " ";

    switch (task.priority) {
        case NORMAL: cout << termcolor::white; break;
        case HIGH: cout << termcolor::blue; break;
        case HIGHER: cout << termcolor::yellow; break;
        case HIGHEST: cout << termcolor::red; break;
    }

    cout.width(45);
    cout << std::left << task.taskname << termcolor::reset;
    if (task.project.length() > 0) {
        cout << termcolor::bold << termcolor::magenta << "@" << task.project << " " << termcolor::reset;
    }

    if (task.tags.length() > 0) {
        cout << termcolor::bold << termcolor::yellow << task.tags << termcolor::reset;
    }
    cout << endl;
}

void showTasks() {
    for (auto it = tasks.begin(); it != tasks.end(); it++) {
        if (it->status != 2) {
            displayTask({ it->ID, it->taskname, it->date, it->status, it->priority, it->project, it->tags });
        }
    }
}

void showTasks(TaskStatus status) {
    for (auto it = tasks.begin(); it != tasks.end(); it++) {
        if (it->status == status || status == ALL) {
            displayTask({ it->ID, it->taskname, it->date, it->status, it->priority, it->project, it->tags });
        }
    }
}

void showLastTasks(int x) {
    int i = 0;
    for (auto it = tasks.begin(); it != tasks.end(); it++) {
        if (i >= x) {
            break;
        }
        displayTask({ it->ID, it->taskname, it->date, it->status, it->priority, it->project, it->tags });
        i++;
    }
}

void showTask(int id) {
    for (auto it = tasks.begin(); it != tasks.end(); it++) {
        if (it->ID == id) {
            displayTask({it->ID, it->taskname, it->date, it->status, it->priority, it->project, it->tags});
            return;
        }
    }

    showError("Nie znaleziono zadania o ID: " + to_string(id) + "!");
}

void removeTask(int id) {
    for (auto it = tasks.begin(); it != tasks.end(); it++) {
        if (it->ID == id) {
            tasks.erase(it);
            return;
        }
    }

    showError("Nie znaleziono zadania o ID: " + to_string(id) + "!");
}

void editTaskStatus(int id, TaskStatus taskStatus) {
    for (auto it = tasks.begin(); it != tasks.end(); it++) {
        if (it->ID == id) {
            it->status = taskStatus;
            return;
        }
    }
    showError("Nie znaleziono zadania o ID: " + to_string(id) + "!");
}

void loadFromFile() {
    FILE *infile;
    infile = fopen(TASKS_FILE, "r");

    if (infile == NULL) {
        cerr << ">> Database loading error!" << endl;
        return;
    }

    Task task;
    while(fread(&task, sizeof(struct Task), 1, infile)) {
        tasks.push_back(task);
    }

    remove(TASKS_FILE);
}

void saveToFile() {
    FILE *outfile;
    outfile = fopen(TASKS_FILE, "w");

    if (outfile == NULL) {
        showError("Database opening error!");
        exit(1);
    }

    for (auto it = tasks.begin(); it != tasks.end(); it++) {
        Task *task = &(*it);
        fwrite(task, sizeof(struct Task), 1, outfile);
    }

    if (&fwrite == nullptr) {
        showError("Saving to database error!");
    }
}

int main(int argc, char** argv) {
    if (argc <= 1) {
        cout << termcolor::bold << termcolor::red << "Usage:" << termcolor::white << " memo param" << endl << endl;
        cout << termcolor::bold << "Jak uzywac memo?" << termcolor::reset << endl;
        cout << " memo add xxxxx - dodaj zadanie" << endl;
        cout << "   --i - zadanie z normalnym priorytetem" << endl;
        cout << "   --ii - zadanie z wysokim priorytetem" << endl;
        cout << "   --iii - zadanie z wyzszym priorytetem" << endl;
        cout << "   --iiii - zadanie z najwyzszym priorytetem" << endl;
        cout << "   --normal - zadanie do zrobienia" << endl;
        cout << "   --done - zadanie wykonane" << endl;
        cout << "   --pending - zadanie w trakcie wykonywania" << endl;
        cout << " memo last xxxx - wyswietlenie xxxx ostatnich zadan" << endl;
        cout << " memo list - wyswietlanie zadan" << endl;
        cout << " memo list all - wyswietlanie zadan (z ukonczonymi)" << endl;
        cout << " memo list done - wyswietlanie zadan ukonczonych" << endl;
        cout << " memo list pending- wyswietlanie zadan w trakcie wykonywania" << endl;
        cout << " memo find xxxx - wyswietlenie zadania od ID" << endl;
        cout << " memo rm yy - usuwanie zadania o ID yy" << endl;
        cout << " memo pending yy - ustawianie statusu zadania na w trakcie wykonywania" << endl;
        cout << " memo done yy - ustawianie statusu zadania na wykonane" << endl;
        cout << " memo edit yy - tryb edycji zadania" << endl;
        cout << " memo i yy - zmiana priorytetu zadania o ID na wysoki" << endl;
        cout << " memo ii yy - zmiana priorytetu zadania o ID na wyzszy" << endl;
        cout << " memo iii yy - zmiana priorytetu zadania o ID na najwyzszy" << endl;
        cout << " memo five - ostatnie piec zadan" << endl;
        cout << " memo actual - aktualne zadania o statusie w trakcie wykonywania" << endl;
        cout << endl;
        cout << "      @projekt" << endl;

        return 0;
    }

    cout << termcolor::blue << termcolor::bold << "------------< Memo >------------" << termcolor::reset << endl << endl;
    loadFromFile();

    // Parsing arguments to list of words
    list<String> words;
    for (int i = 1; i < argc; i++) {
        words.push_back({ argv[i] });
    }

    // For single arg commands
    words.push_back({ "" });

    Command command = T_NONE;
    int wordNumber = 0;

    taskNew.priority = NORMAL;
    taskNew.project = "";
    taskNew.taskname = "";
    taskNew.ID = tasks.front().ID + 1;
    taskNew.status = NONE;
    taskNew.tags = "";

    bool isReady = false;

    for (auto it = words.begin(); it != words.end(); it++) {
        if (wordNumber == 0) {
            //cout << " -- Command: " << it->text << endl;
            if (compareStrings(it->text, "add"))      command = T_ADD;
            if (compareStrings(it->text, "last"))     command = T_LAST;
            if (compareStrings(it->text, "list"))     command = T_LIST;
            if (compareStrings(it->text, "find"))     command = T_FIND;
            if (compareStrings(it->text, "rm"))       command = T_RM;
            if (compareStrings(it->text, "pending"))  command = T_PENDING;
            if (compareStrings(it->text, "done"))     command = T_DONE;
            if (compareStrings(it->text, "edit"))     command = T_EDIT;
            if (compareStrings(it->text, "i"))        command = T_I;
            if (compareStrings(it->text, "ii"))       command = T_II;
            if (compareStrings(it->text, "iii"))      command = T_III;
            if (compareStrings(it->text, "iiii"))     command = T_IIII;
            if (compareStrings(it->text, "five"))     command = T_FIVE;
            if (compareStrings(it->text, "actual"))   command = T_ACTUAL;
        } else {
            if (command == T_ADD) {
                if (startsWithCaseInsensitive(it->text, "@")) {
                    taskNew.project = it->text.substr(1, it->text.length() - 1);
                } else if (compareStrings(it->text, "--iiii")) {
                    taskNew.priority = HIGHEST;
                } else if (compareStrings(it->text, "--iii")) {
                    taskNew.priority = HIGHER;
                } else if (compareStrings(it->text, "--ii")) {
                    taskNew.priority = HIGH;
                } else if (compareStrings(it->text, "--i")) {
                    taskNew.priority = HIGH;
                } else if (compareStrings(it->text, "--normal")) {
                    taskNew.status = NONE;
                } else if (compareStrings(it->text, "--pending")) {
                    taskNew.status = PENDING;
                } else if (compareStrings(it->text, "--done")) {
                    taskNew.status = DONE;
                } else {
                    taskNew.taskname += it->text + " ";
                    isReady = true;
                }
            }

            if (command == T_LAST) {
                int i = 1;
                if (it->text.length() > 0) {
                    i = stoi(it->text);
                }

                showLastTasks(i);
                break;
            }

            if (command == T_FIND) {
                if (it->text.length() > 0) {
                    showTask(stoi(it->text));
                } else {
                    showError("Nie znaleziono ID zadania!");
                }
                break;
            }

            if (command == T_RM) {
                if (it->text.length() > 0) {
                    removeTask(stoi(it->text));
                } else {
                    showError("Nie znaleziono ID zadania!");
                }
                break;
            }

            if (command == T_PENDING) {
                if (it->text.length() > 0) {
                    editTaskStatus(stoi(it->text), PENDING);
                } else {
                    showError("Nie znaleziono ID zadania!");
                }
                break;
            }

            if (command == T_LIST) {
                if (compareStrings(it->text, "")) showTasks();
                if (compareStrings(it->text, "pending")) showTasks(PENDING);
                if (compareStrings(it->text, "done")) showTasks(DONE);
                if (compareStrings(it->text, "all")) showTasks(ALL);
                break;
            }
        }
        wordNumber++;
    }

    // Commit
    if (command == T_ADD && isReady) {
        tasks.push_front(taskNew);
        cout << termcolor::cyan << termcolor::bold << "#" << taskNew.ID << termcolor::reset << " created" << endl;
        //showTasks();
    }

//    tasks.push_back({ 157, "Odblokowac cos", "miesiac temu", NONE, NORMAL });
//    tasks.push_back({ 211, "Zablokowac cos", "jutro", PENDING, NORMAL });
//    tasks.push_back({ 334, "Nie wiem co jeszcze", "jutro", DONE, HIGH });
//    tasks.push_back({ 442, "Siema elo", "za 3 dni", NONE, HIGHER });
//    tasks.push_back({ 587, "Testowanie", "za 10 dni", NONE, HIGHEST });

    cout << endl;

    saveToFile();

    return 0;
}