#include <ostream>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <map>
#include <vector>
#include <fstream>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <unordered_map>

#define UNUSED(x) (void)(x)
#define PATCH_REPORT_FILE "/data/patch_report.txt"
class Commit
{
public:
    Commit(std::string changeid, std::string info, char level=-1):change_id(changeid), mInformation(info), mLevel(level),mIsUnknown(1){};
public:
    std::string change_id;
    std::string mInformation;
    char mLevel;
    char mIsUnknown;
};

class Repo
{
public:
    int patch_count;
    std::string mount_path;
    std::vector<Commit> commits;
};

class PatchStatistics
{
public:
    int repo_count;
    int commit_count;
};

typedef std::map<std::string, Repo> PatchListMap;
static std::vector<std::string> g_white_list;
static int print_patch_level=-1;
#define PRINT_P0 ((print_patch_level == -1) || (print_patch_level == 0))
#define PRINT_P1 ((print_patch_level == -1) || (print_patch_level == 1))
static int FindIn(std::string change_id,std::string repo_path, PatchListMap& changeId_list)
{
    if (repo_path.empty()) {
        for (auto i=changeId_list.begin(); i != changeId_list.end(); i++)
        {
            for (auto j=i->second.commits.begin(); j != i->second.commits.end();j++)
            {
                if (change_id == j->change_id)
                {
                    j->mIsUnknown=0;
                    return 1;
                }
            }
        }
    } else {
        Repo repo = changeId_list[repo_path];
        for (auto k=repo.commits.begin(); k != repo.commits.end(); k++)
        {
            if (change_id == k->change_id)
            {
                k->mIsUnknown=0;
                return 1;
            }
        }
    }

    return 0;
}

static int InWhiteList(std::string change_id)
{
    for (auto id = g_white_list.begin(); id != g_white_list.end();id++)
    {
        if (change_id == *id)
        {
            return 1;
        }
    }

    return 0;
}

static int LoadPatchListFromFile(std::string filename, PatchListMap& repos, PatchStatistics& sta)
{
    std::string line, proj_name;

    std::ifstream patchlist_file(filename);
    Repo repo;
    while (std::getline(patchlist_file, line))
    {
        int line_len=line.length();
        if (line_len > 0) {
            if (line.rfind("repo:", 0) == 0)
            {
                //push to map, and clear repo.commits.
                if (!proj_name.empty())
                {
                    repos[proj_name] = repo;
                    repo.commits.clear();
                }

                char repo_path[256] = {0};
                int patch_count=0;
                int count = std::sscanf(line.c_str(), "repo:%[^:]:%d", repo_path,&patch_count);
                if (count !=2)
                {
                    std::cout<<"error parse repos line"<<std::endl;
                    continue;
                }
                proj_name = repo_path;
                repo.mount_path = repo_path;
                repo.patch_count = patch_count;
                sta.repo_count++;
            }else {
                repo.commits.push_back(Commit(line, ""));
                sta.commit_count++;
            }
        }
    }

    if (!proj_name.empty())
    {
        repos[proj_name] = repo;
    }

    return 0;
}

static int GenPatchReportFromPatchInfo(std::string filename,std::string report, PatchListMap& id_list)
{
    std::string line, proj_name;
    PatchListMap repos;

    int p0_applied_count=0;
    int p0_unapplied_count=0;
    int p1_applied_count=0;
    int p1_unapplied_count=0;
    int other_applied_count=0;
    int other_unapplied_count=0;
    int unknown_count=0;
    int latest_merged_date = -1;
    int latest_release_date = -1;
    std::unordered_map<int, int> p0_unapplied_count_by_date;
    std::unordered_map<int, int> p1_unapplied_count_by_date;
    std::unordered_map<int, int> other_unapplied_count_by_date;

    std::ifstream patchlist_file(filename);
    std::ofstream report_file(report, std::ios_base::out);
    Repo repo;
    PatchStatistics sta;
    while (std::getline(patchlist_file, line))
    {
        int line_len=line.length();
        if (line_len > 0) {
            std::string result="";
            if (line.rfind("repo:", 0) == 0)
            {
                //push to map, and clear repo.commits.
                if (!proj_name.empty())
                {
                    repos[proj_name] = repo;
                    repo.commits.clear();
                }

                char repo_path[256] = {0};
                int patch_count=0;
                int count = std::sscanf(line.c_str(), "repo:%[^:]:%d", repo_path,&patch_count);
                if (count !=2)
                {
                    std::cout<<"error parse repos line"<<std::endl;
                    continue;
                }
                proj_name = repo_path;
                repo.mount_path = repo_path;
                repo.patch_count = patch_count;
                sta.repo_count++;
                report_file<<line<<"\n";
            }else {
                char priority[5] = {0};
                char change_id[256] = {0};
                int date, priority_num;
                int* applied_count;
                int* unapplied_count;
                std::unordered_map<int, int>* unapplied_count_by_date;

                if (std::sscanf(line.c_str(), "%[^:]:%[^:]:%d", priority, change_id, &date) == 3) {
                    if (std::string(priority) == "P0") {
                        priority_num = 0;
                        applied_count = &p0_applied_count;
                        unapplied_count = &p0_unapplied_count;
                        unapplied_count_by_date = &p0_unapplied_count_by_date;
                    } else if (std::string(priority) == "P1") {
                        priority_num = 1;
                        applied_count = &p1_applied_count;
                        unapplied_count = &p1_unapplied_count;
                        unapplied_count_by_date = &p1_unapplied_count_by_date;
                    } else {
                        priority_num = 2;
                        applied_count = &other_applied_count;
                        unapplied_count = &other_unapplied_count;
                        unapplied_count_by_date = &other_unapplied_count_by_date;
                    }

                    repo.commits.push_back(Commit(change_id,line, priority_num));
                    sta.commit_count++;

                    if (date > latest_release_date) {
                        latest_release_date = date;
                    }
                    if (FindIn(change_id,"",id_list)) {
                        result="Y";
                        (*applied_count)++;
                        if (date > latest_merged_date) {
                            latest_merged_date = date;
                        }
                    } else {
                        result="N";
                        if (InWhiteList(change_id)) {
                            (*applied_count)++;
                            if (date > latest_merged_date) {
                                latest_merged_date = date;
                            }
                        } else {
                            (*unapplied_count)++;
                            (*unapplied_count_by_date)[date]++;
                            if ((print_patch_level == -1) || (print_patch_level == priority_num)) {
                                std::cout<<result<<":"<<line<<std::endl;
                            }
                        }
                    }
                    report_file<<result<<":"<<line<<"\n";
                }
            }
        }
    }
    int p0_needapplied_count = 0;
    int p1_needapplied_count = 0;

    for (auto it = p0_unapplied_count_by_date.begin(); it != p0_unapplied_count_by_date.end(); ++it) {
        if (it->first <= latest_merged_date) {
            p0_needapplied_count += it->second;
        }
    }

    for (auto it = p1_unapplied_count_by_date.begin(); it != p1_unapplied_count_by_date.end(); ++it) {
        if (it->first <= latest_merged_date) {
            p1_needapplied_count += it->second;
        }
    }

    int unapplied_patch_p0 = p0_unapplied_count - p0_needapplied_count;
    int unapplied_patch_p1 = p1_unapplied_count - p1_needapplied_count;
    int unapplied_patch_total = unapplied_patch_p0 + unapplied_patch_p1;

    report_file << "Project Latest patch apply date: " << latest_merged_date << ", Aml reference latest released patch date: " << latest_release_date << std::endl;
    std::cout << "Project Latest patch apply date: " << latest_merged_date << ", Aml reference latest released patch date: " << latest_release_date << std::endl;

    report_file << "Patch not applied between " << latest_merged_date << " and " << latest_release_date << ":" << std::endl;
    report_file << "Unapplied patch number(total|P0|P1): " << unapplied_patch_total << " | " << unapplied_patch_p0 << " | " << unapplied_patch_p1 << std::endl;
    report_file << std::endl;

    std::cout << "Patch not applied between " << latest_merged_date << " and " << latest_release_date << ":" << std::endl;
    std::cout << "Unapplied patch number(total|P0|P1): " << unapplied_patch_total << " | " << unapplied_patch_p0 << " | " << unapplied_patch_p1 << std::endl;
    std::cout << std::endl;

    int p0_total_needapply_patch = p0_applied_count + p0_needapplied_count;
    int p1_total_needapply_patch = p1_applied_count + p1_needapplied_count;


    report_file << "Patch applied state before Latest patch apply date:" << std::endl;
    report_file<<"P0(total|applied|unapplied):"<<p0_total_needapply_patch<<" | "<<p0_applied_count<<" | "<<p0_needapplied_count << ", applied rate: " << (p0_applied_count * 100 / p0_total_needapply_patch) << "%" <<std::endl;
    report_file<<"P1(total|applied|unapplied):"<<p1_total_needapply_patch<<" | "<<p1_applied_count<<" | "<<p1_needapplied_count << ", applied rate: " << (p1_applied_count * 100 / p1_total_needapply_patch) << "%" <<std::endl;
    report_file<<"total:"<< p0_total_needapply_patch + p1_total_needapply_patch <<std::endl;
    report_file << std::endl;

    std::cout << "Patch applied state before Latest patch apply date:" << std::endl;
    std::cout<<"P0(total|applied|unapplied):"<<p0_total_needapply_patch<<" | "<<p0_applied_count<<" | "<<p0_needapplied_count << ", applied rate: " << (p0_applied_count * 100 / p0_total_needapply_patch) << "%" <<std::endl;
    std::cout<<"P1(total|applied|unapplied):"<<p1_total_needapply_patch<<" | "<<p1_applied_count<<" | "<<p1_needapplied_count << ", applied rate: " << (p1_applied_count * 100 / p1_total_needapply_patch) << "%"<<std::endl;
    std::cout<<"total:"<< p0_total_needapply_patch + p1_total_needapply_patch <<std::endl;
    std::cout << std::endl;

    int p0_total_patch = p0_applied_count + p0_unapplied_count;
    int p1_total_patch = p1_applied_count + p1_unapplied_count;

    report_file << "Patch applied state compared to the reference latest version total:" << std::endl;
    report_file<<"P0(total|applied|unapplied):"<<p0_total_patch<<" | "<<p0_applied_count<<" | "<<p0_unapplied_count << ", applied rate: " << (p0_applied_count * 100 / p0_total_patch) << "%" <<std::endl;
    report_file<<"P1(total|applied|unapplied):"<<p1_total_patch<<" | "<<p1_applied_count<<" | "<<p1_unapplied_count << ", applied rate: " << (p1_applied_count * 100 / p1_total_patch) << "%" <<std::endl;
    report_file<<"total:"<<p0_total_patch + p1_total_patch<<std::endl;

    std::cout << "Patch applied state compared to the reference latest version total:" << std::endl;
    std::cout<<"P0(total|applied|unapplied):"<<p0_total_patch<<" | "<<p0_applied_count<<" | "<<p0_unapplied_count << ", applied rate: " << (p0_applied_count * 100 / p0_total_patch) << "%" <<std::endl;
    std::cout<<"P1(total|applied|unapplied):"<<p1_total_patch<<" | "<<p1_applied_count<<" | "<<p1_unapplied_count << ", applied rate: " << (p1_applied_count * 100 / p1_total_patch) << "%" <<std::endl;
    std::cout<<"total:"<<p0_total_patch + p1_total_patch<<std::endl;

    report_file<<"Applied unknown ChangeIds:"<<std::endl;
    for (auto i=id_list.begin(); i != id_list.end(); i++)
    {
        for (auto j=i->second.commits.begin(); j != i->second.commits.end();j++)
        {
            if (j->mIsUnknown)
            {
                report_file<<j->change_id<<std::endl;
                unknown_count++;
            }
        }
    }
    report_file<<"Applied unknown ChangeIds list end,total:"<<unknown_count<<std::endl;

    if (!proj_name.empty())
    {
        repos[proj_name] = repo;
    }

    std::cout<<"Patch info count:"<<sta.commit_count<<std::endl;
    return 0;
}

static void usage(int argc, char** argv)
{
    UNUSED(argc);
    std::cout<<argv[0]<<" [-l <0 or 1>] <Change-ID list> <Patch info list> [White list]"<<std::endl;
    std::cout<<"\t[-l <0 or 1>]    : Patch level,0(P0) or 1(P1),default is -1(all)"<<std::endl;
    std::cout<<"\t<Change-ID list> : File name,patch list of this project"<<std::endl;
    std::cout<<"\t<Patch info list>: File name,patch information list provided by Amlogic"<<std::endl;
    std::cout<<"\t[White list]     : File name,lists changeIDs which will be ignored when check"<<std::endl;
}

int main(int argc, char* argv[])
{
    int print_patch_version, opt;
    if (argc < 3)
    {
        usage(argc, argv);
        exit(EXIT_FAILURE);
    }

    print_patch_version = 0;
    while ((opt = getopt(argc, argv, "l:v:")) != -1) {
        switch (opt) {
        case 'l':
            if (strcmp(optarg,"0") == 0) {
                print_patch_level = 0;
            } else if (strcmp(optarg,"1") == 0) {
                print_patch_level = 1;
            } else {
                std::cout<<"invalid patch level "<<optarg<<std::endl;
                usage(argc, argv);
                exit(EXIT_FAILURE);
            }
            break;
        case 'v':
            print_patch_version = 1;
            break;
        default:
            usage(argc, argv);
            exit(EXIT_FAILURE);
        }
    }

    std::cout<<"print_patch_level:"<<print_patch_level<<std::endl;
    if (optind >= argc) {
        usage(argc, argv);
        exit(EXIT_FAILURE);
    }

    if (access(argv[optind], F_OK) != 0 || access(argv[optind+1], F_OK) != 0 )
    {
        std::cout<<argv[optind]<<" or "<<argv[optind+1]<<" not exist."<<std::endl;
        exit(EXIT_FAILURE);
    }
    std::string change_id_list_file = argv[optind];
    std::string patch_info_list_file = argv[optind+1];
    PatchListMap change_id_list;
    PatchStatistics sta_change_id;
    if (optind+2 < argc) {
        std::string white_list_file = argv[optind+2];
        if (access(argv[optind+2], F_OK) == 0) {
            std::cout<<"white_list_file:"<<white_list_file<<std::endl;
            std::ifstream stream_white_list(white_list_file);
            std::string line;
            int count = 0;
            while (std::getline(stream_white_list, line))
            {
                int line_len=line.length();
                if (line_len > 0) {
                    g_white_list.push_back(line);
                    count++;
                }
            }
            std::cout<<"white list count:"<<count<<std::endl;
        } else {
            std::cout<<"white list file \""<<white_list_file<<"\" not exist."<<std::endl;
        }
    }

    std::cout<<"Parsing Change-ID list file "<<change_id_list_file<<"..."<<std::endl;
    LoadPatchListFromFile(change_id_list_file, change_id_list, sta_change_id);
    std::cout<<"Collected Change-ID count:"<<sta_change_id.commit_count<<std::endl;
    std::cout<<"Generating patch report to "<<PATCH_REPORT_FILE<<" from "<<patch_info_list_file<<"..."<<std::endl;
    GenPatchReportFromPatchInfo(patch_info_list_file, PATCH_REPORT_FILE, change_id_list);
    std::cout<<argv[0]<<" Done"<<std::endl;
    return 0;
}
