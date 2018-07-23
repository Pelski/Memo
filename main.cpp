#include <iostream>
#include <ctime>
#include <string>
#include <list>
#include <algorithm>
#include <sstream>
#include "colors.h"

using namespace std;

#define TASKS_FILE "tasks.dat"

enum Command {
    T_NONE,
    T_ADD,
    T_LAST,
    T_LIST,
    T_RM,
    T_NEW,
    T_PENDING,
    T_DONE,
    T_FIND,
    T_EDIT,
    T_I,
    T_II,
    T_III,
    T_IIII,
    T_FIVE,
    T_ACTUAL,
    T_RUN,
    T_EXIT
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

struct Task {
    int ID{};
    string taskName;
    string date;
    int status{};
    int priority{};
    string project;
    string tags;
};

struct String {
    string text;
};

list<Task> tasks;
Task taskNew;

void showError(const string &err) {
    cerr << termcolor::bold << termcolor::red << " >> " << err << termcolor::reset << endl;
}

bool startsWithCaseInsensitive(string mainStr, string toMatch) {
    transform(mainStr.begin(), mainStr.end(), mainStr.begin(), ::tolower);
    transform(toMatch.begin(), toMatch.end(), toMatch.begin(), ::tolower);

    return mainStr.find(toMatch) == 0;
}

bool compareStrings(const string &s1, const string &s2) {
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
        default:break;
    }

    cout << " ";

    switch (task.priority) {
        case NORMAL: cout << termcolor::white; break;
        case HIGH: cout << termcolor::blue; break;
        case HIGHER: cout << termcolor::yellow; break;
        case HIGHEST: cout << termcolor::red; break;
        default:break;
    }

    cout.width(45);
    cout << std::left << task.taskName << termcolor::reset;
    if (task.project.length() > 0) {
        cout << termcolor::bold << termcolor::magenta << "@" << task.project << " " << termcolor::reset;
    }

    if (task.tags.length() > 0) {
        cout << termcolor::bold << termcolor::yellow << task.tags << termcolor::reset;
    }
    cout << endl;
}

void showTasks() {
    for (auto &task : tasks) {
        if (task.status != 2) {
            displayTask({task.ID, task.taskName, task.date, task.status, task.priority, task.project, task.tags });
        }
    }
}

void showTasks(TaskStatus status) {
    for (auto &task : tasks) {
        if (task.status == status || status == ALL) {
            displayTask({task.ID, task.taskName, task.date, task.status, task.priority, task.project, task.tags });
        }
    }
}

void showLastTasks(int x) {
    int i = 0;
    for (auto &task : tasks) {
        if (i >= x) {
            break;
        }
        displayTask({task.ID, task.taskName, task.date, task.status, task.priority, task.project, task.tags });
        i++;
    }
}

void showTask(int id) {
    for (auto &task : tasks) {
        if (task.ID == id) {
            displayTask({task.ID, task.taskName, task.date, task.status, task.priority, task.project, task.tags});
            return;
        }
    }

    showError("Nie znaleziono zadania o ID: " + to_string(id) + " do wyswietlenia!");
}

void removeTask(int id) {
    for (auto it = tasks.begin(); it != tasks.end(); it++) {
        if (it->ID == id) {
            tasks.erase(it);
            return;
        }
    }

    showError("Nie znaleziono zadania o ID: " + to_string(id) + " do usuniecia!");
}

void editTaskStatus(int id, TaskStatus taskStatus) {
    for (auto &task : tasks) {
        if (task.ID == id) {
            task.status = taskStatus;
            displayTask({task.ID, task.taskName, task.date, task.status, task.priority, task.project, task.tags});
            return;
        }
    }
    showError("Nie znaleziono zadania o ID: " + to_string(id) + " do edycji!");
}

void loadFromFile() {
    FILE *infile;
    infile = fopen(TASKS_FILE, "r");

    if (infile == nullptr) {
        showError("Database loading error!");
        return;
    }

    Task it;
    while(fread(&it, sizeof(struct Task), 1, infile)) {
        tasks.push_back(it);
    }

    fclose(infile);
}

void removeFile() {
    remove("backup.dat");
    rename(TASKS_FILE, "backup.dat");
    remove(TASKS_FILE);
}

void saveToFile() {
    FILE *outfile;
    outfile = fopen(TASKS_FILE, "w");

    if (outfile == nullptr) {
        showError("Database opening error!");
        exit(1);
    }

    for (auto &it : tasks) {
        Task *task = &it;
        fwrite(task, sizeof(struct Task), 1, outfile);
    }

    if (&fwrite == nullptr) {
        showError("Saving to database error!");
    }

    fclose(outfile);
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
        cout << " memo new yy - ustawianie statusu zadania na nowe" << endl;
        cout << " memo pending yy - ustawianie statusu zadania na w trakcie wykonywania" << endl;
        cout << " memo done yy - ustawianie statusu zadania na wykonane" << endl;
        cout << " memo edit yy - tryb edycji zadania" << endl;
        cout << " memo i yy - zmiana priorytetu zadania o ID na normalny" << endl;
        cout << " memo ii yy - zmiana priorytetu zadania o ID na wysoki" << endl;
        cout << " memo iii yy - zmiana priorytetu zadania o ID na wyzszy" << endl;
        cout << " memo iiii yy - zmiana priorytetu zadania o ID na najwyzszy" << endl;
        cout << " memo five - ostatnie piec zadan" << endl;
        cout << " memo actual - aktualne zadania o statusie w trakcie wykonywania" << endl;
        cout << endl;
        cout << "      @projekt" << endl;

        return 0;
    }

    bool appExit = true;
    do {
        cout << termcolor::blue << termcolor::bold << "------------< Memo >------------" << termcolor::reset << endl << endl;
        // Loading database
        loadFromFile();

        // Parsing arguments to list of words
        list<String> words;
        if (compareStrings(argv[1], "run")) {
            appExit = false;
            string wordsInput;
            cout << ">> ";
            getline(cin, wordsInput);

            stringstream ssin(wordsInput);
            while (ssin.good()){
                string word;
                ssin >> word;
                words.push_back({ word });
            }
        } else {
            for (int i = 1; i < argc; i++) {
                words.push_back({ argv[i] });
            }
        }

        // For single arg commands
        words.push_back({ "" });

        Command command = T_NONE;
        int wordNumber = -1;

        taskNew.priority = NORMAL;
        taskNew.project = "";
        taskNew.taskName = "";
        taskNew.ID = tasks.front().ID + 1;
        taskNew.status = NONE;
        taskNew.tags = "";

        bool isReady = false;

        for (auto &word : words) {
            wordNumber++;
            if (wordNumber == 0) {
                //cout << " -- Command: " << it->text << endl;
                if (compareStrings(word.text, "add") || compareStrings(word.text, "a"))      command = T_ADD;
                if (compareStrings(word.text, "last") || compareStrings(word.text, "l"))     command = T_LAST;
                if (compareStrings(word.text, "list") || compareStrings(word.text, "ls"))    command = T_LIST;
                if (compareStrings(word.text, "find") || compareStrings(word.text, "s"))     command = T_FIND;
                if (compareStrings(word.text, "rm"))                                         command = T_RM;
                if (compareStrings(word.text, "new") || compareStrings(word.text, "n"))      command = T_NEW;
                if (compareStrings(word.text, "pending") || compareStrings(word.text, "p"))  command = T_PENDING;
                if (compareStrings(word.text, "done") || compareStrings(word.text, "d"))     command = T_DONE;
                if (compareStrings(word.text, "edit") || compareStrings(word.text, "e"))     command = T_EDIT;
                if (compareStrings(word.text, "i"))                                          command = T_I;
                if (compareStrings(word.text, "ii"))                                         command = T_II;
                if (compareStrings(word.text, "iii"))                                        command = T_III;
                if (compareStrings(word.text, "iiii"))                                       command = T_IIII;
                if (compareStrings(word.text, "five") || compareStrings(word.text, "f"))     command = T_FIVE;
                if (compareStrings(word.text, "actual") || compareStrings(word.text, "t"))   command = T_ACTUAL;
                if (compareStrings(word.text, "run"))                                        command = T_RUN;
                if (compareStrings(word.text, "exit"))                                       command = T_EXIT;
            } else {
                if (command == T_ADD || command == T_EDIT) {
                    if (command == T_EDIT) {
                        // ID zadania
                        if (wordNumber == 1) {
                            if (word.text.length() > 0) {
                                int id = stoi(word.text);
                                bool found = false;
                                for (auto &task : tasks) {
                                    if (task.ID == id) {
                                        taskNew.ID = task.ID;
                                        taskNew.taskName = "";
                                        taskNew.date = task.date;
                                        taskNew.status = task.status;
                                        taskNew.priority = task.priority;
                                        taskNew.project = task.project;
                                        taskNew.tags = task.tags;
                                        found = true;
                                        break;
                                    }
                                }

                                if (!found) {
                                    showError("Nie znaleziono zadania o podanym ID!");
                                }
                            } else {
                                showError("Nie znaleziono ID zadania!");
                                break;
                            }
                            continue;
                        }
                    }

                    if (startsWithCaseInsensitive(word.text, "@")) {
                        taskNew.project = word.text.substr(1, word.text.length() - 1);
                    } else if (compareStrings(word.text, "--iiii")) {
                        taskNew.priority = HIGHEST;
                    } else if (compareStrings(word.text, "--iii")) {
                        taskNew.priority = HIGHER;
                    } else if (compareStrings(word.text, "--ii")) {
                        taskNew.priority = HIGH;
                    } else if (compareStrings(word.text, "--i")) {
                        taskNew.priority = HIGH;
                    } else if (compareStrings(word.text, "--normal")) {
                        taskNew.status = NONE;
                    } else if (compareStrings(word.text, "--pending")) {
                        taskNew.status = PENDING;
                    } else if (compareStrings(word.text, "--done")) {
                        taskNew.status = DONE;
                    } else {
                        taskNew.taskName += word.text + " ";
                        isReady = true;
                    }
                }

                if (command == T_LAST) {
                    int i = 1;
                    if (word.text.length() > 0) {
                        i = stoi(word.text);
                    }

                    showLastTasks(i);
                    break;
                }

                if (command == T_FIND) {
                    if (word.text.length() > 0) {
                        showTask(stoi(word.text));
                    } else {
                        showError("Nie znaleziono ID zadania!");
                    }
                    break;
                }

                if (command == T_RM) {
                    if (word.text.length() > 0) {
                        removeTask(stoi(word.text));
                    } else {
                        showError("Nie znaleziono ID zadania!");
                    }
                    break;
                }

                if (command == T_NEW) {
                    if (word.text.length() > 0) {
                        editTaskStatus(stoi(word.text), NONE);
                    } else {
                        showError("Nie znaleziono ID zadania!");
                    }
                    break;
                }

                if (command == T_PENDING) {
                    if (word.text.length() > 0) {
                        editTaskStatus(stoi(word.text), PENDING);
                    } else {
                        showError("Nie znaleziono ID zadania!");
                    }
                    break;
                }

                if (command == T_DONE) {
                    if (word.text.length() > 0) {
                        editTaskStatus(stoi(word.text), DONE);
                    } else {
                        showError("Nie znaleziono ID zadania!");
                    }
                    break;
                }

                if (command == T_LIST) {
                    if (compareStrings(word.text, "")) showTasks();
                    if (compareStrings(word.text, "pending")) showTasks(PENDING);
                    if (compareStrings(word.text, "done")) showTasks(DONE);
                    if (compareStrings(word.text, "all")) showTasks(ALL);
                    break;
                }
            }
        }

        // Commit
        if (command == T_ADD && isReady) {
            tasks.push_front(taskNew);
            cout << termcolor::cyan << termcolor::bold << "#" << taskNew.ID << termcolor::reset << " created" << endl;
        }

        if (command == T_EDIT && isReady) {
            removeTask(taskNew.ID);
            tasks.push_front(taskNew);
            cout << termcolor::cyan << termcolor::bold << "#" << taskNew.ID << termcolor::reset << " edited" << endl;
            showTask(taskNew.ID);
        }

        if (command == T_RUN) {
            appExit = false;
        }

        if (command == T_EXIT) {
            appExit = true;
        }

        cout << endl;

        removeFile();
        saveToFile();

        tasks.clear();
    } while (!appExit);

    return 0;
}