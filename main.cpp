
#include <memory>
#include <iostream>
#include <sstream>
#include <vector>
#include <git2.h>

const char* const yellow = "\x1B[33m";
const char* const blue = "\x1B[34m";
const char* const red = "\x1B[31m";
const char* const green = "\x1B[32m";
const char* const reset = "\x1B[0m";

struct branch
{
    std::string name;
    git_branch_t type;
    bool head;
};

/*
struct gitref
{
    git_reference *ref;
    gitref() :ref(p){}
    gitref() = delete;
    ~gitref()
    {
        git_reference_free(ref);
    }

};
*/

// iterate over a repo returning a vector of branches
void gitBranches(git_repository* repo, std::vector<branch>& v)
{
    git_branch_iterator *it;

    if (!git_branch_iterator_new(&it, repo, GIT_BRANCH_ALL))
    {
        git_reference *ref;
        git_branch_t type;

        branch headBranch{};

      while (!git_branch_next(&ref, &type, it))
      {

         const char* name;
        if (!git_branch_name(&name, ref))
        {
            bool isHead{false};
            git_branch_t type{GIT_BRANCH_LOCAL};

            git_reference *out;
            int rc = git_branch_lookup(&out, repo, name, GIT_BRANCH_LOCAL);
            if (rc != 0)
            {
                git_reference_free(out);
                rc = git_branch_lookup(&out, repo, name, GIT_BRANCH_REMOTE);
                if (rc == 0)
                    type = GIT_BRANCH_REMOTE;
            }
            if (rc == 0) {

                isHead =  git_branch_is_head(out) == 1 ? true  : false;

                git_reference_free(out);

                if (isHead)
                {
                    headBranch.name = name;
                    headBranch.head = true;
                    headBranch.type = type;
                }
                else
                  //  v.push_back(branch{name:name, type:type, head:isHead});
                    v.insert(v.begin(), branch{name:name, type:type, head:isHead});
            }
        }

        git_reference_free(ref);

      }
      v.insert(v.begin(), headBranch);
      git_branch_iterator_free(it);
    }
}

std::string branchstring(const git_oid& oid, git_repository* repo)
{
    std::vector<branch> v;
    gitBranches(repo, v);
    std::ostringstream str;

    for (auto b : v)
    {
        const char* color = (b.type == GIT_BRANCH_LOCAL) ? green  : red;

        if (b.head)
            str << " (" << blue << "HEAD ->" <<reset << green << b.name;
        else
            str << ", " << color << b.name << reset << "";
    }

    str << ")";
    return str.str();
}

std::string datestring(const git_time *intime)
{
    //std::ostringstream str;

    char sign, out[32], buff[50];
    struct tm *intm;
    int offset, hours, minutes;
    time_t t;

    offset = intime->offset;
    if (offset < 0) {
        sign = '-';
        offset = -offset;
    } else {
        sign = '+';
    }

    hours   = offset / 60;
    minutes = offset % 60;

    t = (time_t)intime->time + (intime->offset * 60);

    intm = gmtime(&t);
    strftime(out, sizeof(out), "%a %b %e %T %Y", intm);

    sprintf(buff, "%s %c%02d%02d\n", out, sign, hours, minutes);

    return std::string(buff);
}

int main(int _argc, char ** _argv) {

    if (_argc < 2)
        return 1; // do usage statement

    git_libgit2_init();


    // open repo
    git_repository * repo = nullptr;
    git_repository_open(&repo, _argv[1]);

    //create a repo walker
    git_revwalk * walker = nullptr;
    git_revwalk_new(&walker, repo);
    git_revwalk_sorting(walker, GIT_SORT_NONE);

    // set to HEAD
    git_revwalk_push_head(walker);

    git_oid oid;

    bool head = true;
    while(!git_revwalk_next(&oid, walker))
    {
        git_commit * commit = nullptr;
        git_commit_lookup(&commit, repo, &oid);

        /// commit
        std::cout   << yellow << "commit " << git_oid_tostr_s(&oid) << reset;
         if (head ) {std::cout << branchstring(oid, repo); head = false;}
         std::cout << std:: endl;

        /// Author
        const git_signature *sig = git_commit_author(commit);
        if (sig != nullptr)
        {
            std::cout << "Author: " << sig->name << " " << sig->email << std::endl;
        /// Date
           std::cout << "Date:   " << datestring(&sig->when) << std::endl;

        /// commit message
           std::cout << "   " << git_commit_summary(commit)<< std::endl <<std::endl;
        }

        git_commit_free(commit);
    }

    git_libgit2_shutdown();
    return 0;
}