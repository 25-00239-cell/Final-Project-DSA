#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <algorithm>
#include <ctime>

using namespace std;

// ─────────────────────────────────────────────
//  Data Structure
// ─────────────────────────────────────────────
struct Entry {
    string date;
    string time;
    int    mood;       // 1–5
    string moodLabel;
    string note;
};

// ─────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────
string getCurrentDate() {
    time_t now = time(nullptr);
    tm* t = localtime(&now);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", t);
    return string(buf);
}

string getCurrentTime() {
    time_t now = time(nullptr);
    tm* t = localtime(&now);
    char buf[6];
    strftime(buf, sizeof(buf), "%H:%M", t);
    return string(buf);
}

string moodToLabel(int m) {
    switch (m) {
        case 1: return "Terrible";
        case 2: return "Bad";
        case 3: return "Okay";
        case 4: return "Good";
        case 5: return "Great";
        default: return "Unknown";
    }
}

string moodEmoji(int m) {
    switch (m) {
        case 1: return "[1] Terrible";
        case 2: return "[2] Bad";
        case 3: return "[3] Okay";
        case 4: return "[4] Good";
        case 5: return "[5] Great";
        default: return "[?]";
    }
}

void printLine(char c = '-', int n = 50) {
    cout << string(n, c) << "\n";
}

void printHeader(const string& title) {
    printLine('=');
    cout << "  " << title << "\n";
    printLine('=');
}

// ─────────────────────────────────────────────
//  File I/O
// ─────────────────────────────────────────────
const string DATA_FILE = "journal_entries.csv";

void saveEntry(const Entry& e) {
    ofstream f(DATA_FILE, ios::app);
    if (!f) { cerr << "Error: Cannot open file.\n"; return; }
    // Escape pipe in note just in case
    string safeNote = e.note;
    replace(safeNote.begin(), safeNote.end(), '|', '/');
    f << e.date << "|" << e.time << "|" << e.mood << "|" << e.moodLabel << "|" << safeNote << "\n";
}

vector<Entry> loadEntries() {
    vector<Entry> entries;
    ifstream f(DATA_FILE);
    if (!f) return entries;
    string line;
    while (getline(f, line)) {
        if (line.empty()) continue;
        istringstream ss(line);
        Entry e;
        string moodStr;
        getline(ss, e.date,      '|');
        getline(ss, e.time,      '|');
        getline(ss, moodStr,     '|');
        getline(ss, e.moodLabel, '|');
        getline(ss, e.note);
        e.mood = stoi(moodStr);
        entries.push_back(e);
    }
    return entries;
}

// ─────────────────────────────────────────────
//  Features
// ─────────────────────────────────────────────
void addEntry() {
    printHeader("New Journal Entry");
    Entry e;
    e.date = getCurrentDate();
    e.time = getCurrentTime();
    cout << "  Date : " << e.date << "\n";
    cout << "  Time : " << e.time << "\n\n";

    cout << "  How are you feeling today?\n";
    for (int i = 1; i <= 5; i++) cout << "    " << moodEmoji(i) << "\n";
    cout << "\n  Enter mood (1-5): ";

    while (!(cin >> e.mood) || e.mood < 1 || e.mood > 5) {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "  Invalid. Enter a number between 1 and 5: ";
    }
    cin.ignore(1000, '\n');
    e.moodLabel = moodToLabel(e.mood);

    cout << "\n  Write your journal note (press Enter when done):\n  > ";
    getline(cin, e.note);
    if (e.note.empty()) e.note = "(no note)";

    saveEntry(e);
    cout << "\n  Entry saved! Mood: " << e.moodLabel << "\n";
    printLine();
}

void viewEntries() {
    vector<Entry> entries = loadEntries();
    printHeader("All Journal Entries");
    if (entries.empty()) {
        cout << "  No entries found. Start by adding one!\n";
        printLine();
        return;
    }
    for (const auto& e : entries) {
        cout << "  " << e.date << " " << e.time
             << "  |  Mood: " << e.moodLabel << " (" << e.mood << "/5)\n"
             << "  Note: " << e.note << "\n";
        printLine('-', 50);
    }
}

void analyzePatterns() {
    vector<Entry> entries = loadEntries();
    printHeader("Mood Pattern Analysis");
    if (entries.size() < 2) {
        cout << "  Need at least 2 entries for analysis.\n";
        printLine();
        return;
    }

    // Count each mood
    map<string, int> moodCount;
    int total = 0;
    double sum = 0;
    for (const auto& e : entries) {
        moodCount[e.moodLabel]++;
        sum += e.mood;
        total++;
    }

    double avg = sum / total;

    cout << "  Total entries  : " << total << "\n";
    cout << fixed << setprecision(2);
    cout << "  Average mood   : " << avg << " / 5.00\n\n";
    cout << "  Mood Breakdown:\n";
    for (int i = 5; i >= 1; i--) {
        string label = moodToLabel(i);
        int count = moodCount.count(label) ? moodCount[label] : 0;
        int bar = (total > 0) ? (count * 20 / total) : 0;
        cout << "    " << setw(8) << left << label << " [" << string(bar, '#') << string(20-bar, ' ') << "] " << count << "\n";
    }

    // Most common mood
    auto best = max_element(moodCount.begin(), moodCount.end(),
        [](const pair<string,int>& a, const pair<string,int>& b){ return a.second < b.second; });
    cout << "\n  Most frequent mood: " << best->first << " (" << best->second << " times)\n";
    printLine();
}

void generateReport() {
    vector<Entry> entries = loadEntries();
    printHeader("Mental Wellness Summary Report");
    if (entries.empty()) {
        cout << "  No entries to report yet.\n";
        printLine();
        return;
    }

    int total = entries.size();
    double sum = 0;
    int best = 1, worst = 5;
    string bestDate, worstDate;

    for (const auto& e : entries) {
        sum += e.mood;
        if (e.mood > best)  { best = e.mood;  bestDate  = e.date; }
        if (e.mood < worst) { worst = e.mood; worstDate = e.date; }
    }

    double avg = sum / total;
    string firstDate = entries.front().date;
    string lastDate  = entries.back().date;

    cout << "  Period         : " << firstDate << " to " << lastDate << "\n";
    cout << "  Total entries  : " << total << "\n";
    cout << fixed << setprecision(2);
    cout << "  Average mood   : " << avg << " / 5.00\n";
    cout << "  Best mood day  : " << bestDate  << " (" << moodToLabel(best)  << ")\n";
    cout << "  Lowest mood day: " << worstDate << " (" << moodToLabel(worst) << ")\n\n";

    // Simple wellness message
    cout << "  Wellness Insight:\n  ";
    if (avg >= 4.0)      cout << "You've been doing great! Keep up the positive energy.\n";
    else if (avg >= 3.0) cout << "You're doing okay. Small steps lead to big improvements.\n";
    else                 cout << "Tough times don't last. Consider talking to someone you trust.\n";

    // Export to text file
    ofstream report("wellness_report.txt");
    if (report) {
        report << "===== Mental Wellness Report =====\n";
        report << "Period        : " << firstDate << " to " << lastDate << "\n";
        report << "Total entries : " << total << "\n";
        report << fixed << setprecision(2);
        report << "Average mood  : " << avg << " / 5.00\n";
        report << "Best day      : " << bestDate  << " (" << moodToLabel(best)  << ")\n";
        report << "Lowest day    : " << worstDate << " (" << moodToLabel(worst) << ")\n";
        cout << "\n  Report saved to 'wellness_report.txt'\n";
    }
    printLine();
}

// ─────────────────────────────────────────────
//  Main Menu
// ─────────────────────────────────────────────
int main() {
    cout << "\n";
    printLine('=');
    cout << "   MOOD & JOURNAL TRACKER\n";
    cout << "   Your daily mental wellness companion\n";
    printLine('=');

    int choice = 0;
    do {
        cout << "\n  MENU\n";
        printLine('-', 30);
        cout << "  1. Add today's entry\n";
        cout << "  2. View all entries\n";
        cout << "  3. Analyze mood patterns\n";
        cout << "  4. Generate wellness report\n";
        cout << "  5. Exit\n";
        printLine('-', 30);
        cout << "  Choice: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(1000, '\n');
            choice = -1;
        }
        cin.ignore(1000, '\n');
        cout << "\n";

        switch (choice) {
            case 1: addEntry();        break;
            case 2: viewEntries();     break;
            case 3: analyzePatterns(); break;
            case 4: generateReport();  break;
            case 5: cout << "  Goodbye! Take care of yourself.\n\n"; break;
            default: cout << "  Invalid choice. Try again.\n"; break;
        }
    } while (choice != 5);

    return 0;
}