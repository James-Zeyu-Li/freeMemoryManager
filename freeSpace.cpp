#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <cstdlib>
#include <string>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <iomanip>
#include <getopt.h>

using namespace std;

enum class Policy { BEST, WORST, FIRST};
enum class Order {ADDRSORT, SIZESORT_ASC, SIZESORT_DES};

struct Job{
    vector<int> operation;
};

struct Malloc {
    int size;
    int start;
    int headerSize;
    Policy policy;
    Order order;
    bool coalesce;
    vector<pair<int, int>> freelist;
    map<int, pair<int, int>> sizeMap;
    int counter = 0;

    Malloc(int size, int start, int headerSize, Policy policy, Order order, bool coalesce)
        : size(size), start(start), headerSize(headerSize), policy(policy),
          order(order), coalesce(coalesce), counter(0) {
        freelist.push_back({start, size});
    }
};

void addToMap(Malloc &m, int address, int sizeWithHead) {
    if (m.sizeMap.find(address) == m.sizeMap.end()) {
        m.sizeMap[address] = sizeWithHead;
    }
}

// The function to dump the free list
void dumpFreeList(const Malloc &m) {
    cout << "Free List [Size: " << m.freelist.size() << "]: ";
    for (const auto &block : m.freelist) {
        cout << "[addr: " << block.first << " size: " << block.second << "] ";
    }
    cout << endl;
}

void displayAllocatedBlocks(const Malloc &m) {
    cout << "Allocated Blocks: [";
    for (const auto &entry : m.sizeMap) {
        cout << "[Address: " << entry.first << ", Size: " << entry.second << "] ";
    }
    cout << "]" << endl;
}


// Find the address and size to be allocated according to the policy
std::pair<int, int> allocate(Malloc &m, int request) {
    int bestIdx = -1;
    int bestSize = -1;
    int count = 0;

    int sizeWithHead = request + m.headerSize;

    cout << "Attempting to allocate " << request << " (including header: " << sizeWithHead << ")" << endl;
    dumpFreeList(m);

    // set best address if the policy selection is BEST
    if (m.policy == Policy::BEST)     {
        bestSize = m.size + 1;
    }
    else     {
        bestSize = -1;
    }

    // Find the best address and best size according to the policy
    for (int i = 0; i < m.freelist.size(); i++)     {
        count++;
        int address = m.freelist[i].first;
        int size = m.freelist[i].second;

        if (size >= sizeWithHead) {
            if ((m.policy == Policy::BEST && size < bestSize) ||
                (m.policy == Policy::WORST && size > bestSize)){
                bestSize = size;
                bestIdx = i;
            }

            if (m.policy == Policy::FIRST){
                break;
            }
        }
    }

    if (bestIdx != -1) {
        int bestAddress = m.freelist[bestIdx].first;
        int bestSize = m.freelist[bestIdx].second;

        if (bestSize > sizeWithHead){
            int updatedFreeAddress = m.freelist[bestIdx].first += sizeWithHead;
            int updatedFreeSize = m.freelist[bestIdx].second -= sizeWithHead;
        }
        else if (bestSize == sizeWithHead) {
            m.freelist.erase(m.freelist.begin() + bestIdx);
        }
        else {
            cerr << "There is no memory block that fits the current request" << endl;
            abort();
        }

        m.sizeMap[m.counter] = {bestAddress, sizeWithHead};
        cout << "Allocated at address " << bestAddress << " with ID " << m.counter << endl;
        m.counter++;

        addToMap(m, bestAddress, sizeWithHead);
        cout << "Allocated at address " << bestAddress << endl;
        dumpFreeList(m);
        displayAllocatedBlocks(m);

        return {bestAddress, count};
    }
    cout << "Failed to allocate " << request << " after checking " << count << " blocks" << endl;

    return {-1, count};
}

// The function to free a block of memory
int free(Malloc &m, int blockID) {
    cout << "Attempting to free block with ID " << blockID << endl;

    if (m.sizeMap.find(blockID) == m.sizeMap.end()) {
        cout << "Invalid free: no such block exists" << endl;
        return -1;
    }

    int address = m.sizeMap[blockID].first;
    int size = m.sizeMap[blockID].second;
    m.freelist.push_back({address, size});

    if (m.order == Order::ADDRSORT) {
        sort(m.freelist.begin(), m.freelist.end());
    } else if (m.order == Order::SIZESORT_ASC) {
        sort(m.freelist.begin(), m.freelist.end(), [](auto &a, auto &b) {
            return a.second < b.second;
        });
    } else if (m.order == Order::SIZESORT_DES) {
        sort(m.freelist.begin(), m.freelist.end(), [](auto &a, auto &b) {
            return a.second > b.second;
        });
    }

    m.sizeMap.erase(blockID);

    if (m.coalesce) {
        vector<pair<int, int>> newFreelist;
        pair<int, int> curr = m.freelist[0];
        for (int i = 1; i < m.freelist.size(); i++) {
            if (m.freelist[i].first == (curr.first + curr.second)) {
                curr.second += m.freelist[i].second;
            } else {
                newFreelist.push_back(curr);
                curr = m.freelist[i];
            }
        }
        newFreelist.push_back(curr);
        m.freelist = newFreelist;
    }

    cout << "Freed block with ID " << blockID << endl;
    return 0;
}


struct Options
{
    int size = 100;
    int start = 1000;
    int headerSize = 0;
    Policy policy = Policy::BEST;
    Order order = Order::ADDRSORT;
    bool coalesce = false;
    vector<Job> jobList = {
        Job{vector<int>{+7, -0}}}; //, +5, +4, +9, -2, -1, -3, +80
};


void showHelp() {
    std::cout << "Usage:\n"
              << "-S,         size of the heap\n"
              << "-B,         base address of the heap\n"
              << "-H,         size of the header\n"
              << "-P,         list search policy (BEST, WORST, FIRST)\n"
              << "-l,         list order (ADDRSORT, SIZESORT+, SIZESORT-)\n"
              << "-C,         coalesce the free list?\n"
              << "-A,         list of operations (+10,-0,etc)\n"
              << "-h,         show this help message and exit\n";
}



void Abort(const string &str)
{
    cerr << "Error: " << str << endl;
    exit(1);
}

void splitStringToInt(const string &str, char delimiter, vector<int> &out)
{
    stringstream ss(str);
    string token;
    while (getline(ss, token, delimiter))
    {
        try
        {
            int value = stoi(token); 
            out.push_back(value);
        }
        catch (invalid_argument &e)
        {
            cerr << "Invalid number in list: " << token << endl;
            exit(1);
        }
        catch (out_of_range &e)
        {
            cerr << "Number out of range: " << token << endl;
            exit(1);
        }
    }
}

Options parseOptions(int argc, char *argv[])
{
    Options opts;

    int opt;
    const char *short_opts = "S:B:H:p:l:CA:h";

    while ((opt = getopt(argc, argv, short_opts)) != -1)
    {
        switch (opt)
        {
        case 'S':
            if (optarg != nullptr)
            {
                try
                {
                    opts.size = stoi(optarg);
                }
                catch (invalid_argument &e)
                {
                    Abort("Invalid heap size: " + string(optarg));
                }
            }
            else
            {
                Abort("Heap size not provided after -S");
            }
            break;
        case 'B':
            if (optarg != nullptr)
            {
                try
                {
                    opts.start = stoi(optarg);
                }
                catch (invalid_argument &e)
                {
                    Abort("Invalid base address: " + string(optarg));
                }
            }
            else
            {
                Abort("Base address not provided after -B");
            }
            break;
        case 'H':
            if (optarg != nullptr)
            {
                try
                {
                    opts.headerSize = stoi(optarg);
                }
                catch (invalid_argument &e)
                {
                    Abort("Invalid header size: " + string(optarg));
                }
            }
            else
            {
                Abort("Header size not provided after -H");
            }
            break;
        case 'p':
            if (optarg != nullptr)
            {
                string policyStr = optarg;
                if (policyStr == "BEST")
                    opts.policy = Policy::BEST;
                else if (policyStr == "WORST")
                    opts.policy = Policy::WORST;
                else if (policyStr == "FIRST")
                    opts.policy = Policy::FIRST;
                else
                    Abort("Invalid policy: " + policyStr);
            }
            else
            {
                Abort("Policy not provided after -p");
            }
            break;
        case 'l':
            if (optarg != nullptr)
            {
                string orderStr = optarg;
                if (orderStr == "ADDRSORT")
                    opts.order = Order::ADDRSORT;
                else if (orderStr == "SIZESORT+")
                    opts.order = Order::SIZESORT_ASC;
                else if (orderStr == "SIZESORT-")
                    opts.order = Order::SIZESORT_DES;
                else
                    Abort("Invalid order: " + orderStr);
            }
            else
            {
                Abort("Order not provided after -l");
            }
            break;
        case 'C':
            opts.coalesce = true;
            break;
        case 'A':
            if (optarg != nullptr)
            {
                Job job;
                splitStringToInt(optarg, ',', job.operation);
                opts.jobList.push_back(job);
            }
            else
            {
                Abort("Operation list not provided after -A");
            }
            break;
        case 'h':
            showHelp();
            exit(0);
        default:
            showHelp();
            exit(1);
        }
    }

    return opts;
}


int main(int argc, char const *argv[]) {
    Options opts = parseOptions(argc, (char **)argv);

    Malloc m(opts.size, opts.start, opts.headerSize, opts.policy, opts.order, opts.coalesce);

    for (auto &job : opts.jobList) {
        for (auto &operation : job.operation) {
            if (operation > 0) {
                auto [address, count] = allocate(m, operation);
                if (address == -1) {
                    cout << "Failed to allocate " << operation << " in " << count << " searches \n" << endl;
                } else {
                    cout << "Allocated " << operation << " at " << address << " in " << count << " searches \n" << endl;
                }
            } else {
                free(m, -operation);
                cout << "Freed " << -operation << endl;
            }
        }
    }

    return 0;
}
