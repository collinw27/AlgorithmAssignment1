#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <deque>
#include <chrono>
#include <fstream>

using namespace std;

/*
 *  For testing in terminal:
 *  Match mode:
 *      ex. AlgorithmAssignment1.exe match .\example.in .\example.out
 *  Verify mode:
 *      ex. AlgorithmAssignment1.exe verify .\example.in .\example.out
 * 
 *  Running without arguments defaults to match with no file inputs
 *  File arguments can be replaced with * to use terminal for input/output
 *  TIMED can be added as a final argument to time the code in ns
 *  (used for scalability testing)
 *
 */

struct Matching
{
    int hospital;
    int student;
};

// make vectors for preferences
struct Instance {
    int n = 0;

    vector<vector<int>> hospPref;
    vector<vector<int>> studPref;
    vector<vector<int>> studRank;
};

// parsing and validation helpers
static bool isPermutation1toN(const vector<int>& line, int n) {
    if ((int)line.size() != n) return false;
    vector<char> seen(n + 1, 0);
    for (int v : line) {
        if (v < 1 || v > n) return false;
        if (seen[v]) return false;
        seen[v] = 1;
    }
    return true;
}

static bool readInstance(istream& in, Instance& inst, string& err) {
    //cout << "readInst";
    int n;
    if (!(in >> n)) {
        err = "EMPTY_OR_MISSING_N";
        return false;
    }
    if (n < 0) {
        err = "INVALID_N_NEGATIVE";
        return false;
    }

    inst.n = n;
    inst.hospPref.assign(n + 1, vector<int>(n + 1, 0));
    inst.studPref.assign(n + 1, vector<int>(n + 1, 0));
    inst.studRank.assign(n + 1, vector<int>(n + 1, 0));

    // Hospitals
    for (int h = 1; h <= n; h++) {
        vector<int> line;
        line.reserve(n);
        for (int k = 1; k <= n; k++) {
            int v;
            if (!(in >> v)) {
                err = "TRUNCATED_HOSPITAL_PREFS";
                return false;
            }
            inst.hospPref[h][k] = v;
            line.push_back(v);
        }
        if (!isPermutation1toN(line, n)) {
            err = "INVALID_HOSPITAL_PREF_LINE_" + to_string(h);
            return false;
        }
    }

    // Students
    for (int s = 1; s <= n; s++) {
        vector<int> line;
        line.reserve(n);
        for (int k = 1; k <= n; k++) {
            int v;
            if (!(in >> v)) {
                err = "TRUNCATED_STUDENT_PREFS";
                return false;
            }
            inst.studPref[s][k] = v;
            line.push_back(v);
        }
        if (!isPermutation1toN(line, n)) {
            err = "INVALID_STUDENT_PREF_LINE_" + to_string(s);
            return false;
        }
        for (int k = 1; k <= n; k++) {
            int h = inst.studPref[s][k];
            inst.studRank[s][h] = k;
        }
    }

    return true;
}

// class for the Matching Engine
class MatchingEngine
{
    unsigned int count;
    Instance inst;
    
public:

    // Expected # of hospitals and students must be assigned at creation

    explicit MatchingEngine(unsigned int count) : count(count) {
        inst.n = (int)count;
        inst.hospPref.assign(count + 1, vector<int>(count + 1, 0));
        inst.studPref.assign(count + 1, vector<int>(count + 1, 0));
        inst.studRank.assign(count + 1, vector<int>(count + 1, 0));
    }

    void set_hospital_preferences(int hospital, const vector<int>& preferences) {
        if ((unsigned)preferences.size() != count)
            throw invalid_argument("Incorrect number of preferences (hospital).");
        if (hospital < 1 || hospital > (int)count)
            throw invalid_argument("Hospital id out of range.");
        if (!isPermutation1toN(preferences, (int)count))
            throw invalid_argument("Hospital preferences must be a permutation of 1..n.");

        for (int k = 1; k <= (int)count; k++)
            inst.hospPref[hospital][k] = preferences[k - 1];
    }

    void set_student_preferences(int student, const vector<int>& preferences) {
        if ((unsigned)preferences.size() != count)
            throw invalid_argument("Incorrect number of preferences (student).");
        if (student < 1 || student > (int)count)
            throw invalid_argument("Student id out of range.");
        if (!isPermutation1toN(preferences, (int)count))
            throw invalid_argument("Student preferences must be a permutation of 1..n.");

        for (int k = 1; k <= (int)count; k++) {
            int h = preferences[k - 1];
            inst.studPref[student][k] = h;
            inst.studRank[student][h] = k;
        }
    }

    // new solve() approach
    // returns hospital -> student mapping (1-indexed) and proposal count
    pair<vector<int>, long long> solve() {
        deque<int> unmatched_hospitals;
        vector<int> next_choices(count + 1, 1);
        vector<int> student_matches(count + 1, 0);
        vector<int> hospital_matches(count + 1, 0);

        for (int h = 1; h <= (int)count; h++)
            unmatched_hospitals.push_back(h);

        long long proposals = 0;

        while (!unmatched_hospitals.empty()) {
            int hospital = unmatched_hospitals.front();

            // in case of bad input (shouldn’t happen with complete lists)
            if (next_choices[hospital] > (int)count) {
                unmatched_hospitals.pop_front();
                continue;
            }

            int student = inst.hospPref[hospital][next_choices[hospital]];
            next_choices[hospital]++;
            proposals++;

            if (student_matches[student] == 0) {
                // student free -> match
                student_matches[student] = hospital;
                hospital_matches[hospital] = student;
                unmatched_hospitals.pop_front();
            } else {
                int prev_hospital = student_matches[student];

                // student prefers lower rank
                if (inst.studRank[student][hospital] < inst.studRank[student][prev_hospital]) {
                    // student switches
                    student_matches[student] = hospital;
                    hospital_matches[hospital] = student;

                    hospital_matches[prev_hospital] = 0;

                    unmatched_hospitals.pop_front();
                    unmatched_hospitals.push_back(prev_hospital);
                }
                // else rejected; hospital stays unmatched and tries again later
            }
        }

        return {hospital_matches, proposals};
    }

};

// Verifier (done as a separate mode rather than a separate program. could be changed later)
static string verifyMatching(const Instance& inst, const vector<pair<int,int>>& pairs) {
    int n = inst.n;
    if ((int)pairs.size() != n) {
        return "INVALID: expected " + to_string(n) + " matching lines, got " + to_string(pairs.size());
    }

    vector<int> hospToStud(n + 1, 0), studToHosp(n + 1, 0);
    vector<char> seenHosp(n + 1, 0), seenStud(n + 1, 0);

    // validity
    for (auto [h, s] : pairs) {
        if (h < 1 || h > n || s < 1 || s > n)
            return "INVALID: out-of-range pair (" + to_string(h) + "," + to_string(s) + ")";
        if (seenHosp[h]) return "INVALID: hospital " + to_string(h) + " appears more than once";
        if (seenStud[s]) return "INVALID: student " + to_string(s) + " appears more than once";
        seenHosp[h] = 1; seenStud[s] = 1;
        hospToStud[h] = s;
        studToHosp[s] = h;
    }
    for (int h = 1; h <= n; h++) if (!seenHosp[h]) return "INVALID: hospital " + to_string(h) + " is unmatched";
    for (int s = 1; s <= n; s++) if (!seenStud[s]) return "INVALID: student " + to_string(s) + " is unmatched";

    // hospital ranks
    vector<vector<int>> hospRank(n + 1, vector<int>(n + 1, 0));
    for (int h = 1; h <= n; h++)
        for (int k = 1; k <= n; k++)
            hospRank[h][inst.hospPref[h][k]] = k;

    // stability (blocking pair)
    for (int h = 1; h <= n; h++) {
        int sMatched = hospToStud[h];
        int rankMatched = hospRank[h][sMatched];

        for (int k = 1; k < rankMatched; k++) {
            int s = inst.hospPref[h][k];
            int hMatchedToS = studToHosp[s];
            if (inst.studRank[s][h] < inst.studRank[s][hMatchedToS]) {
                return "UNSTABLE: blocking pair (hospital " + to_string(h) + ", student " + to_string(s) + ")";
            }
        }
    }

    return "VALID STABLE";
}

static vector<pair<int,int>> readMatchingPairs(istream& in) {
    vector<pair<int,int>> pairs;
    int h, s;
    while (in >> h >> s) pairs.push_back({h, s});
    return pairs;
}

int main(int argc, char** argv) {

    // arguments
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string mode = (argc >= 2 ? string(argv[1]) : "match");
    string file1 = (argc >= 3 ? string(argv[2]) : "*");
    string file2 = (argc >= 4 ? string(argv[3]) : "*");
    bool timed_mode = (argc >= 5 && string(argv[4]) == "TIMED");

    // start timer
    auto begin = chrono::steady_clock::now();

    // match mode
    if (mode == "match") {
        Instance inst;
        string err;

        ifstream stream1;
        if (file1 != "*")
            stream1 = ifstream(file1);
        if (!readInstance((file1 == "*") ? cin : stream1, inst, err)) {
            cout << "INVALID: " << err << "\n";
            return 0;
        }
        if (inst.n == 0) return 0;

        MatchingEngine engine(inst.n);

        for (int h = 1; h <= inst.n; h++) {
            vector<int> prefs;
            prefs.reserve(inst.n);
            for (int k = 1; k <= inst.n; k++) prefs.push_back(inst.hospPref[h][k]);
            engine.set_hospital_preferences(h, prefs);
        }
        for (int s = 1; s <= inst.n; s++) {
            vector<int> prefs;
            prefs.reserve(inst.n);
            for (int k = 1; k <= inst.n; k++) prefs.push_back(inst.studPref[s][k]);
            engine.set_student_preferences(s, prefs);
        }

        auto [hospToStud, proposals] = engine.solve();

        ofstream stream2;
        if (file2 != "*")
            stream2 = ofstream{file2};
        ostream& outputStream = (file2 == "*") ? cout : stream2;
        for (int h = 1; h <= inst.n; h++) {
            outputStream << h << " " << hospToStud[h] << "\n";
        }

        // print time elapsed, if necessary
        if (timed_mode) {
            auto end = std::chrono::steady_clock::now();
            cout << "Elapsed: " << chrono::duration_cast<chrono::microseconds>(end-begin).count() << " ns" << endl;
        }

        return 0;
    }

    // verify mode
    if (mode == "verify") {
        Instance inst;
        string err;

        // read either from input file or terminal
        ifstream stream1;
        if (file1 != "*")
            stream1 = ifstream(file1);
        if (!readInstance((file1 == "*") ? cin : stream1, inst, err)) {
            cout << "INVALID: " << err << "\n";
            return 0;
        }

        // read either from output file or terminal
        ifstream stream2;
        if (file2 != "*")
            stream2 = ifstream(file2);
        auto pairs = readMatchingPairs((file2 == "*") ? cin : stream2);
        cout << verifyMatching(inst, pairs) << "\n";
        
        // print time elapsed, if necessary
        if (timed_mode) {
            auto end = std::chrono::steady_clock::now();
            cout << "Elapsed: " << chrono::duration_cast<chrono::microseconds>(end-begin).count() << " ns" << endl;
        }
        
        return 0;
    }

    // invalid mode
    cerr << "Unknown mode: " << mode << endl << endl;
    cerr 
        << "For testing in terminal:" << endl
        << "  Match mode:" << endl
        << "    ex. AlgorithmAssignment1.exe match .\\example.in .\\example.out" << endl
        << "  Verify mode:" << endl
        << "    ex. AlgorithmAssignment1.exe verify .\\example.in .\\example.out" << endl
        << "" << endl
        << "  Running without arguments defaults to `match * *`" << endl
        << "  File arguments can be replaced with * to use terminal for input/output" << endl
        << "  TIMED can be added as a final argument to time the code in ns" << endl
        ;
    return 1;
}