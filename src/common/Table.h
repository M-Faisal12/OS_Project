#pragma once
#include <iostream>
#include <cstring>

struct Row
{
    char Catagory[20] = {'\0'};
    int total = 0;
};

class Table
{
    Row row[50];
    int i = 0;

public:
    Row *getrow()
    {
        return row;
    }
    bool empty()
    {
        return i == 0;
    }

    bool full()
    {
        return i == 50;
    }

    void insert(const char cat[], int val)
    {
        int j;

        if ((j = find(cat)) >= 0)
        {
            row[j].total += val;
            return;
        }

        if (full())
            return;

        strncpy(row[i].Catagory, cat, sizeof(row[i].Catagory) - 1);
        row[i].Catagory[sizeof(row[i].Catagory) - 1] = '\0';

        row[i].total = val;
        i++;
    }

    int size()
    {
        return i;
    }

    int find(const char cat[])
    {
        for (int j = 0; j < i; j++)
        {
            if (strcmp(row[j].Catagory, cat) == 0)
            {
                return j;
            }
        }
        return -1;
    }

    int getTotal(const char cat[])
    {
        int idx = find(cat);

        if (idx == -1)
            return 0;

        return row[idx].total;
    }

    // 🔥 PRINT FUNCTION ADDED
    void print()
    {
        std::cout << "\n===== AGGREGATION TABLE =====\n";

        for (int j = 0; j < i; j++)
        {
            std::cout << row[j].Catagory
                      << " : "
                      << row[j].total
                      << std::endl;
        }

        std::cout << "=============================\n";
    }
};