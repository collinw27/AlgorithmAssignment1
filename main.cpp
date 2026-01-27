#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <initializer_list>
#include <iostream>
#include <stdexcept>

struct Matching
{
    std::string hospital;
    std::string student;
};

class MatchingEngine
{

    unsigned int count;
    std::unordered_map<std::string, std::string*> hospitals;
    std::unordered_map<std::string, std::string*> students;
    
public:

    // Expected # of hospitals and students must be assigned at creation

    MatchingEngine(unsigned int count)
    {
        this->count = count;
    }

    void set_hospital_preferences(std::string hospital, std::initializer_list<std::string> preferences)
    {
        // Error on redefinition or mismatched list type

        if (preferences.size() != count)
            throw std::invalid_argument("Incorrect number of preferences.");
        if (hospitals.find(hospital) != hospitals.end())
            throw std::invalid_argument("Redefinition of hospital.");

        // Copy into array within the map

        std::string* arr = new std::string[count];
        std::copy(preferences.begin(), preferences.end(), arr);
        hospitals[hospital] = arr;
    }

    void set_student_preferences(std::string student, std::initializer_list<std::string> preferences)
    {
        if (preferences.size() != count)
            throw std::invalid_argument("Incorrect number of preferences.");
        if (students.find(student) != students.end())
            throw std::invalid_argument("Redefinition of student.");
        std::string* arr = new std::string[count];
        std::copy(preferences.begin(), preferences.end(), arr);
        students[student] = arr;
    }

    std::vector<Matching>* solve()
    {
        // Make sure there are correct # of hospitals/students

        if (hospitals.size() != count || students.size() != count)
            throw std::invalid_argument("Incorrect number of hospitals/students.");

        // Read in data

        std::vector<std::string> unmatched_hospitals;
        std::unordered_map<std::string, int> next_choices; // the next student a hospital will choose
        std::unordered_map<std::string, std::string> student_matches; // maps student -> hospital

        for (auto pair : hospitals)
        {
            unmatched_hospitals.push_back(pair.first);
            next_choices[pair.first] = 0;
        }
        for (auto pair : students)
        {
            student_matches[pair.first] = "";
        }

        // Keep checking while an unmatched hospital remains
        // Basically iterates over the map until a full iteration
        // passes without a new matching being necessary

        while (!unmatched_hospitals.empty())
        {
            // Get next student preference

            std::string hospital = unmatched_hospitals.front();
            std::string student = hospitals[hospital][next_choices[hospital]];
            next_choices[hospital]++;

            // Match if student is free

            if (student_matches[student].empty())
            {
                student_matches[student] = hospital;
                unmatched_hospitals.erase(unmatched_hospitals.begin());
            }

            // If student is matched, check if they would prefer this hospital
            // If they don't prefer it, nothing happens

            else
            {
                std::string* prefs = students[student];
                std::string prev_hospital = student_matches[student];
                int prev_pref = -1;
                int new_pref = -1;
                for (int i = 0; i < count; i++)
                {
                    if (prefs[i] == hospital)
                        new_pref = i;
                    else if (prefs[i] == prev_hospital)
                        prev_pref = i;
                }
                if (new_pref < prev_pref) // Lower preference is better
                {
                    student_matches[student] = hospital;
                    unmatched_hospitals.erase(unmatched_hospitals.begin());
                    unmatched_hospitals.push_back(prev_hospital);
                }
            }
        }

        // Convert matches dictionary to list format

        auto output = new std::vector<Matching>;
        for (auto pair : student_matches)
        {
            output->push_back(Matching{pair.first, pair.second});
        }
        return output;
    }
};

int main()
{
    MatchingEngine list(3);
    list.set_hospital_preferences("X", {"a", "b", "c"});
    list.set_hospital_preferences("Y", {"a", "b", "c"});
    list.set_hospital_preferences("Z", {"b", "c", "a"});
    list.set_student_preferences("a", {"Y", "X", "Z"});
    list.set_student_preferences("b", {"X", "Y", "Z"});
    list.set_student_preferences("c", {"X", "Y", "Z"});

    std::vector<Matching>* output = list.solve();
    for (Matching matching : *output)
    {
        std::cout << matching.hospital << " -> " << matching.student << std::endl;
    }

    return 0;
}