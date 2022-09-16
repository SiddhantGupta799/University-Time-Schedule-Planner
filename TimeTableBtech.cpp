#include <bits/stdc++.h>
#include <unordered_map>
#include <unordered_set>
using namespace std;
#include "PyBasics.h"
using namespace Py;

// Faculty and Subjects that they teach List
unordered_map<string, string> sub_fac_list;

// Semester and its Subjects
unordered_map<int, unordered_set<string>> sem_sub_list;

// Subject and its Total Hours per week
unordered_map<string, int> sub_hours_list;

// Subject and its Type 
unordered_map<string, char> sub_type;

unordered_map<string, bool> faculty_availability;			// log to maintain a check for faculty availability

// table to maintain a log that a subject for a particular day doesn't get more than two hours and if 
// one session existed in one half another session doesn't come up in another half
unordered_map<string, vector<int>> sub_assignment_table;

void check(string) {
	for (auto& p : sub_fac_list) {
		cout << p.first << " : " << p.second << endl;
	}cout << endl;
}

void check(int) {
	for (auto& p : sem_sub_list) {
		cout << p.first << ",[";
		for (auto& v : p.second) {
			cout << v << ",";
		}cout << ']' << endl;
	}cout << endl;
}

void check() {
	for (auto& f : sub_hours_list)
		cout << f.first << " : " << f.second << endl;
	cout << endl;
}

vector<											// Batch 
	vector<										// Day
	vector<										// Hour
	pair<string, string>						// teacher and subject
	>>> timetables;


auto check_practical = [](string s) {
	Lower(s);
	if (s.find("lab") < s.size() or s.find("workshop") < s.size()) return true;
	else return false;
};

auto check_project = [](string s) {
	Lower(s);
	if (s.find("project") < s.size()) return true;
	else return false;
};


void parse_and_initialize(string file, int sem) {
	vector<string> type_sub_hours = Split(file, '\n',false);
	for (auto& l : type_sub_hours) {
		vector<string> temp = Split(l, ',',false);
		if (temp.size() == 3) {
			int hours = Int(temp[2]);
			sub_hours_list[temp[1]] = hours;
			sub_type[temp[1]] = temp[0][0];
			sem_sub_list[sem].insert(temp[1]);
		}
	}
}


void parse_and_initialize_all(vector<string>& files) {
	// subject type, semester and hours initialization
	int semester = 1;
	for (auto& file : files) {
		parse_and_initialize(file, semester);
		semester++;
	}
}


void parse_and_initialize_all(string faculty_sub) {
	vector<string> fields = Split(faculty_sub, '\n', false);
	for (auto& field : fields) {
		vector<string> temp = Split(field, ',', false);
		if (temp.size() == 2) {
			sub_fac_list[temp[1]] = temp[0];
			faculty_availability[temp[0]] = true;
		}
	}
}


void fill_sub_assignment() {
	// filling the sub_assignment_table
	for (int i = 8; i >= 1; i--) {
		for (auto& sub : sem_sub_list[i]) {
			sub_assignment_table[sub] = vector<int>(6, 0);
		}
	}
}


int init(vector<string>&files, string& faculty_subs, bool odd) {
	// Parsing
	parse_and_initialize_all(files);
	parse_and_initialize_all(faculty_subs);

	// Sewing these lists amply to deal with the making of time tables 
	// allocating space to fill timetable
	timetables = vector<vector<vector<pair<string, string>>>>
		(9, vector<vector<pair<string, string>>>
			(6, vector<pair<string, string>>(6)
				)
			);

	fill_sub_assignment();
	

	auto make_available = []() {
		for (auto& p : faculty_availability) p.second = true;
	};

	auto check = [](int batch, int day, int hours = 1, int half = 0) -> pair<string, string> {
		/*
		This function takes a subject relevant to the batch, finds if the faculty for the respective
		subject is available and if subject hours haven't been fulfilled only then it assigns a subject
		on the time table.

		features:
			- Subject hours fulfilled
			- No clash in faculty shifts
			- Practical Subjects in Second Half
			- a subject in particular should be assigned at max 2 hours for a day.
			- a lab or a workshop should be assigned 3 hours at max
			- projects can be assigned 4 hours at max
		*/
		for (auto& sub : sem_sub_list[batch]) {

			string faculty = sub_fac_list[sub];

			if (check_project(sub) and half and
				sub_hours_list[sub] > 0 and
				faculty_availability[faculty] and
				sub_assignment_table[sub][day] < 4
				) {
				faculty_availability[faculty] = false;
				sub_assignment_table[sub][day]++;
				sub_hours_list[sub] -= hours;
				return { sub,faculty };
			}
			else if (sub_type[sub] == 'P' or sub_type[sub] == 'p') {	// means if its second half? go for a Workshop or Lab
				if (half and
					sub_hours_list[sub] > 0 and
					faculty_availability[faculty] and
					sub_assignment_table[sub][day] < 3
					) {
					faculty_availability[faculty] = false;
					sub_assignment_table[sub][day]++;
					sub_hours_list[sub] -= hours;
					return { sub,faculty };
				}
			}
			else {
				if (sub_hours_list[sub] > 0 and
					faculty_availability[faculty] and
					sub_assignment_table[sub][day] < 2
					) {
					sub_assignment_table[sub][day]++;		// hours are added to restrict assignment for non practical subjects to 2 hours
					faculty_availability[faculty] = false;
					sub_hours_list[sub] -= hours;
					return { sub, faculty };
				}
			}
		}
	};


	for (int day = 0; day < 5; day++) {
		for (int hour = 0; hour < 6; hour++) {
			make_available();
			for (int batch = odd? 7:8; batch >= 1; batch -= 2) {
				auto p = check(batch, day, 1, hour > 2);
				timetables[batch][day][hour] = p;
			}
		}
	}

	for (int sem = odd ? 7 : 8; sem >= 1; sem -= 2) {
		cout << "Semester: " << sem << endl;
		for (int day = 0; day < 5; day++) {
			for (int hour = 0; hour < 6; hour++) {
				cout << day + 1 << " : " << timetables[sem][day][hour].first << " : " << timetables[sem][day][hour].second << endl;
			}cout << endl;
		}cout << endl;
	}

	int check_fulfillment = 0;
	for (auto& sub : sub_hours_list) {
		check_fulfillment += sub.second;
	}

	if (check_fulfillment) { cout << "Error"; cin.get(); }

	return 0;
}

string load_file(string path) {
	string data = File(path, READ).readfile();
	if (data == File().EOL()) throw(4);
	return data;
}

vector<string> load_files(string& path) {
	vector<string> file_data;
	cout << "Path set to: " << path << endl;
	cout << "Attempting to Load Files..." << endl;;

	int check = 1;

	for (int i = 1; i <= 8; i++) {
		string temp = "loading " + Str(i) + ".csv...";
		try {
			cout << temp << endl;;
			string data = load_file(path + "/" + Str(i) + ".csv");
			file_data.push_back(data);
			check *= 1;
		}
		catch (...) {
			check *= 0;
			cout << "Error " << temp << endl;;
		}
	}

	if (check) return file_data;
	else throw(-1);
}

int main() {
	char response = 'N';
	int everythings_fine = 1;

	print("Welcome to University Time Schedule Planner");
	print("Time Schedule Planner for B.Tech or any 4 Years Programme with 8 Semesters");
	print("\n\nThe Input Format:");
	print("1-8 should be .csv containing semester-wise subject and hours distribution.\nNamed as 1.csv, 2.csv and so on...");
	print("Last file named as faculty.csv\n\n");
	string path = "";
	path = Input("Provide the Path to all the Files: ");

	vector<string> files;
	if (everythings_fine)
		while (true) {
			try { files = load_files(path); break; }
			catch (int e) {
				print("There was an error loading one or more files");
				response = Lower(Input("Try again? (Y/N): "))[0];
				if (response == 'y') {
					response = Lower(Input("Do you want to change the path? (Y/N): "))[0];
					if (response == 'y') { path = Input("New Path: "); }
				}
				else { everythings_fine = 0; break; }
			}
		}
	else print("Trouble Fetching Data"), exit(0);

	if (everythings_fine) {
		int i = 1;
		for (auto& s : files) {
			cout << "Semester: " << i << endl;
			cout << s << endl << endl;
			i++;
		}
	}

	string faculty_sub;
	if (everythings_fine)
		while (true) {
			try {
				faculty_sub = load_file(path + "/" + "faculty.csv");
				break;
			}
			catch (int e) {
				print("There was an error loading the file faculty.csv.");
				response = Lower(Input("Try again? (Y/N): "))[0];
				if (response == 'y') {
					response = Lower(Input("Do you want to change the path? (Y/N): "))[0];
					if (response == 'y') { path = Input("New Path: "); }
				}
				else { everythings_fine = 0; break; }
			}
		}
	else print("Trouble Fetching Data"), exit(0);

	if (everythings_fine)
		cout << faculty_sub << endl;

	if (everythings_fine) {
		try {
			
			/*parse_and_initialize_all(files);
			parse_and_initialize_all(faculty_sub);
			for (int i = 1; i < 9; i++) {
				for (auto& sub : sem_sub_list[i]) {
					cout << sub << ", " << endl;
				}cout << endl << endl;
			}*/
			
			response = Lower(Input("Is Current Semester Odd? (Y/N): "))[0];
			init(files, faculty_sub, response == 'y');

		}
		catch (...) {
			
		}
	}

	return 0;
}