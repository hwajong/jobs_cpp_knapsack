#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

class Citem
{
private:
	int group_id;	// item의group_id
	int item_id;	// item의id number

	int weight;		// item의weight
	int profit;		// item의profit

	// 다른 members 혹은 member functions (이후는 public 으로 선언 가능)
public:
	void set_group_id(int id) { group_id = id; }
	void set_item_id(int id) { item_id = id; }
	void set_weight(int w) { weight = w; }
	void set_profit(int p) { profit = p; }

	int get_group_id() { return group_id; }
	int get_item_id() { return item_id; }
	int get_weight() { return weight; }
	int get_profit() { return profit; }

	// item 출력을 위한 string 리턴
	string to_str()
	{
		char buff[1024];
		sprintf(buff, "%d %d %d %d", group_id, item_id, weight, profit);
		return buff;
	}
};

class Cknapsack
{
private:
	vector <Citem> my_item_list;	// item 을저장한다
	int item_num;					// 주어진item의개수
	int capacity;					// 주어진knapsack의capacity
	int best_objective_value;		// best objective value (찾아야할값임)

	// 다른 members 혹은 member functions (이후는 public 으로 선언 가능)
	int max_group_id;  // 그룹아이디중 가장 큰 아이디 값 저장

	int cache_size;
	int* cache;        // 중간 계산 결과를 저장하기 위한 변수
	vector<int>* cache_selected_vec;

	vector<int> best_selected_index_vec; // 최종 선택된 item 의 index 저장

	int get_best_objective_value()
	{
		return best_objective_value;
	}

	// 스트링 공백 문자 제거
	void trim(char* s)
	{
		char* p = s;
		int l = strlen(p);

		while(l-1 > 0 && !(p[l-1] & 0x80) && isspace(p[l-1])) p[--l] = 0;
		while(*p && !(*p & 0x80) && isspace(*p)) ++p, --l;

		memmove(s, p, l + 1);
	}

	// Recursive Dynamic Programming 
	int f(int i, int y, vector<int>& selected_index_vec)
	{
		if(y <= 0) return 0;

		// 계산했던 결과는 재사용한다.
		int cache_val = cache[i*capacity + y];
		if(cache_val != -1)
		{
			printf("cache hit - (%d,2%d)\n", i, y);
			selected_index_vec = cache_selected_vec[i*capacity + y];
			return cache_val;
		}

		int optimal_value = 0;
		int optimal_selected = -1;
		vector<int> optimal_selected_vec;

		// exit case
		if(i == max_group_id)
		{
			optimal_value = optimal_value_at_last(y, selected_index_vec);
		}
		else
		{
			for(int j=0; j<(int)my_item_list.size(); j++)
			{
				Citem& item = my_item_list[j];
				if(item.get_group_id() != i) continue;

				vector<int> selected_u;
				vector<int> selected_v;
				int u = f(i+1, y, selected_u);
				int v = 0;
				if(y - item.get_weight() >= 0)
				{
					v = f(i+1, y - item.get_weight(), selected_v) + item.get_profit();
				}

				//cout << endl;
				//printf("f(%d,%2d) = %d\n", i+1, y, u);
				//printf("f(%d,%2d) + %d = %d\n", i+1, y-item.get_weight(), item.get_profit(), v);

				int w = u;
				vector<int>& selected_w = selected_u;

				if(u < v)
				{
					w = v;
					selected_w = selected_v;
				}

				if(optimal_value < w)
				{
					optimal_value = w;
					optimal_selected = j;
					optimal_selected_vec = selected_w;
				}
			}

			selected_index_vec = optimal_selected_vec;
			selected_index_vec.push_back(optimal_selected);
			//printf("selected index : %d\n", optimal_selected);

		}

		// 중간 결과값 저장
		cache[i*capacity + y] = optimal_value;
		cache_selected_vec[i*capacity + y] = selected_index_vec;
		//printf("f(%d,%2d) = %d\n", i, y, optimal_value);

		return optimal_value;
	}

	// 마지막 케이스에서 optimal value 를 구한다.
	int optimal_value_at_last(int remained_capa, vector<int>& selected_index_vec) 
	{
		int optimal_value = 0;
		int optimal_selected_index = 0;
		for(int i=0; i<(int)my_item_list.size(); i++)
		{
			Citem& item = my_item_list[i];
			if(item.get_group_id() != max_group_id) continue;

			if(item.get_weight() <= remained_capa)
			{
				if(item.get_profit() > optimal_value)
				{
					optimal_value = item.get_profit();
					optimal_selected_index = i;
				}
			}
		}

		if(optimal_value > 0)
		{
			selected_index_vec.push_back(optimal_selected_index);
		}
		return optimal_value;
	}


public:

	// destructor - 메모리 해제
	~Cknapsack() 
	{
		if(cache) 
		{
			delete [] cache;
			delete [] cache_selected_vec;
		}
	}

	// solv 결과를 출력파일에 쓴다.
	bool write_to_outfile(const char* outfile)
	{
		ofstream ofs(outfile);
		if(!outfile)
		{
			return false;
		}

		ofs << "group_no, item_no, weight, profit" << endl;

		int total_weight = 0;
		int total_profit = 0;
		//cout << "best_selected_index_vec.size : " << best_selected_index_vec.size() << endl;
		for(int i=best_selected_index_vec.size()-1; i>=0; i--)
		{
			int index = best_selected_index_vec[i];
			//cout << index << endl;
			Citem& item = my_item_list[index];
			ofs << item.to_str() << endl;
			total_weight += item.get_weight();
			total_profit += item.get_profit();
		}

		ofs << endl;
		ofs << "capacity : " << capacity << endl;
		ofs << "total weight : " << total_weight << endl;
		ofs << "total profit : " << total_profit << endl;
		ofs.close();

		// 디버깅을 위한 출력
		cout << endl;
		cout << "---------------------\n";
		cout << "n : " << max_group_id << endl;
		cout << "c : " << capacity << endl;
		cout << "G I W P\n";
		cout << "---------------------\n";

		for(int i=0; i<(int)my_item_list.size(); i++)
		{
			cout << my_item_list[i].to_str() << endl;
		}
		cout << "---------------------\n";

		int v = get_best_objective_value(); 
		cout << "* Best Optimal Value : " << v << endl;

		return true;
	}

	// 해를 구하기 위한 public 메소드
	void solv()
	{
		best_objective_value = f(1, capacity, best_selected_index_vec);
	}

	// 초기화
	// 파일 입력 처리
	// 케시 메모리 생성
	bool init(const string& fname, int capacity)
	{
		if(capacity <= 0) return false;

		this->capacity = capacity;
		best_objective_value = 0;

		// read file data
		ifstream ifs(fname);

		if(!ifs.is_open()) return false;

		int line_count = 0;
		stringstream ss;
		while(!ifs.eof())
		{
			char buff[1024];
			ifs.getline(buff, sizeof(buff));

			// 첫라인 skip
			if(line_count++ == 0) continue;

			// 공백문자 제거
			trim(buff);

			if(strlen(buff) == 0) continue;

			// 파싱하기 위하여 스트링 복사
			ss << buff;

			Citem item;
			for(int i=0; i<4; i++) 
			{
				if(ss.eof()) return false;

				ss.getline(buff, 5, ',');
				//cout << atoi(buff) << " ";

				int n = atoi(buff);

				if(i == 0) item.set_group_id(n);
				else if(i == 1) item.set_item_id(n);
				else if(i == 2) item.set_weight(n);
				else if(i == 3) item.set_profit(n);
			}

			my_item_list.push_back(item);
			max_group_id = item.get_group_id();

			ss.clear();

			//cout << endl;
		}

		ifs.close();

		item_num = my_item_list.size();

		// 중간결과값 저장을 위한 메모리 할당
		// 계산을 편하게 하기위해 +1 씩 더 할당 한다.
		// -1 로 초기화
		cache_size = (max_group_id+1) * (capacity+1);
		cache = new int[cache_size];
		cache_selected_vec = new vector<int>[cache_size];
		for(int i=0; i<cache_size; i++)
		{
			cache[i] = -1; 
		}

		return true;
	}
};


// 프로그램 사용법 출력
void usage(const char* progname)
{
	printf("* usage : %s input_file capacity output_file\n", progname);
}


int main(int argc, char** argv)
{
	// 입력 파라미터 체크
	if(argc != 4)
	{
		usage(argv[0]);
		return 0;
	}

	int capacity = atoi(argv[2]);

	Cknapsack sack;

	// 초기화
	if(!sack.init(argv[1], capacity))
	{
		cout << "** Error - fail to init Cknapsack\n";
		return 0;
	}

	// 해 구하기
	sack.solv();

	// 결과 출력파일에 쓰기
	if(!sack.write_to_outfile(argv[3]))
	{
		cout << "** Error - fail to write outfile\n";
		return 0;
	}

	return 0;
}

