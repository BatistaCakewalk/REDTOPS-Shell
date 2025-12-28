#include "../headers/fs_commands.hpp"
#include "../../core/header/TerminalRenderer.hpp"
#include "../../core/header/Exceptions.hpp" // Include custom exceptions
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <system_error>
#include <unistd.h>    // for getuid()
#include <pwd.h>
#include <cstdlib>

namespace fs = std::filesystem;

// ---------- Helpers ----------

// Expands ~ to home directory
static std::string ExpandHome(const std::string& path) {
    if (!path.empty() && path[0] == '~') {
        const char* home = std::getenv("HOME");
        
        if (!home) {
            struct passwd *pw = getpwuid(getuid());
            if (pw) home = pw->pw_dir;
        }
        if (home) return std::string(home) + path.substr(1);
    }
    return path;
}

// Converts fs::perms to a string like rwxr-xr--
static std::string PermString(fs::perms p) {
    std::string s = "---------";
    s[0] = ((p & fs::perms::owner_read) != fs::perms::none) ? 'r' : '-';
    s[1] = ((p & fs::perms::owner_write) != fs::perms::none) ? 'w' : '-';
    s[2] = ((p & fs::perms::owner_exec) != fs::perms::none) ? 'x' : '-';
    s[3] = ((p & fs::perms::group_read) != fs::perms::none) ? 'r' : '-';
    s[4] = ((p & fs::perms::group_write) != fs::perms::none) ? 'w' : '-';
    s[5] = ((p & fs::perms::group_exec) != fs::perms::none) ? 'x' : '-';
    s[6] = ((p & fs::perms::others_read) != fs::perms::none) ? 'r' : '-';
    s[7] = ((p & fs::perms::others_write) != fs::perms::none) ? 'w' : '-';
    s[8] = ((p & fs::perms::others_exec) != fs::perms::none) ? 'x' : '-';
    return s;
}

// Formats a time_t into YYYY-MM-DD HH:MM
static std::string FormatTime(std::time_t t) {
    std::tm tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M");
    return oss.str();
}

// ---------- pwd ----------
void PwdCommand::Execute(const std::vector<std::string>&) {
    try {
        auto p = fs::current_path();
        TerminalRenderer::Instance().PrintLine(p.string());
    } catch (const fs::filesystem_error& e) {
        throw RedTops::CommandError(std::string("pwd: ") + e.what());
    }
}

// ---------- cd ----------
void CdCommand::Execute(const std::vector<std::string>& args) {
    std::string target;
    if (args.empty()) {
        const char* home = std::getenv("HOME");
        if (!home) {
            struct passwd *pw = getpwuid(getuid());
            if (pw) home = pw->pw_dir;
        }
        if (home) target = home;
        else {
            throw RedTops::CommandError("cd: HOME not set");
        }
    } else {
        target = ExpandHome(args[0]);
    }

    try {
        fs::path p(target);
        if (p.is_relative()) p = fs::current_path() / p;
        fs::current_path(fs::weakly_canonical(p));
    } catch (const fs::filesystem_error& e) {
        throw RedTops::CommandError(std::string("cd: ") + e.what());
    }

    // --- refresh prompt ---
    //TerminalRenderer::Instance().PrintLine(""); // force a newline
    //std::cout << TerminalRenderer::Instance().GetPrompt() << std::flush;
}


// ---------- ls ----------
void LsCommand::Execute(const std::vector<std::string>& args) {
    bool show_all = false;
    bool long_listing = false;
    fs::path target = fs::current_path();

    // parse simple flags: -a, -l (combinations allowed)
    std::vector<std::string> paths;
    for (auto &a : args) {
        if (a.size() > 1 && a[0] == '-') {
            for (size_t i = 1; i < a.size(); ++i) {
                if (a[i] == 'a') show_all = true;
                else if (a[i] == 'l') long_listing = true;
            }
        } else {
            paths.push_back(ExpandHome(a));
        }
    }
    if (!paths.empty()) target = fs::path(paths[0]);

    try {
        if (target.is_relative()) target = fs::current_path() / target;
        if (!fs::exists(target)) {
            throw RedTops::CommandError("ls: target does not exist: " + target.string());
        }

        if (!fs::is_directory(target)) {
            std::ostringstream line;
            if (long_listing) {
                auto st = fs::status(target);
                auto sz = fs::is_regular_file(target) ? fs::file_size(target) : 0;
                auto perms = PermString(st.permissions());
                auto ftime = fs::last_write_time(target);
                auto sctp = std::chrono::system_clock::to_time_t(std::chrono::file_clock::to_sys(ftime));
                line << perms << " " << std::setw(8) << sz << " " << FormatTime(sctp) << " " << target.filename().string();
                TerminalRenderer::Instance().PrintLine(line.str());
            } else {
                TerminalRenderer::Instance().PrintLine(target.filename().string());
            }
            return;
        }

        for (auto &entry : fs::directory_iterator(target)) {
            auto name = entry.path().filename().string();
            if (!show_all && !name.empty() && name[0] == '.') continue;

            if (long_listing) {
                auto st = entry.status();
                std::ostringstream line;
                auto sz = fs::is_regular_file(entry.status()) ? fs::file_size(entry.path()) : 0;
                auto perms = PermString(st.permissions());
                auto ftime = fs::last_write_time(entry.path());
                auto sctp = std::chrono::system_clock::to_time_t(std::chrono::file_clock::to_sys(ftime));
                line << perms << " " << std::setw(8) << sz << " " << FormatTime(sctp) << " " << name;
                TerminalRenderer::Instance().PrintLine(line.str());
            } else {
                TerminalRenderer::Instance().PrintLine(name);
            }
        }
    } catch (const fs::filesystem_error& e) {
        throw RedTops::CommandError(std::string("ls: ") + e.what());
    }
}

// ---------- cat ----------
void CatCommand::Execute(const std::vector<std::string>& args) {
    if (args.empty()) {
        throw RedTops::CommandError("cat: missing file operand");
    }
    fs::path p(ExpandHome(args[0]));
    if (p.is_relative()) p = fs::current_path() / p;

    try {
        if (!fs::exists(p)) {
            throw RedTops::CommandError("cat: file does not exist: " + p.string());
        }
        if (fs::is_directory(p)) {
            throw RedTops::CommandError("cat: cannot display a directory: " + p.string());
        }
        std::ifstream ifs(p, std::ios::binary);
        std::string line;
        while (std::getline(ifs, line)) {
            TerminalRenderer::Instance().PrintLine(line);
        }
    } catch (const fs::filesystem_error& e) {
        throw RedTops::CommandError(std::string("cat: ") + e.what());
    }
}

// ---------- mkdir ----------
void MkdirCommand::Execute(const std::vector<std::string>& args) {
    if (args.empty()) {
        throw RedTops::CommandError("mkdir: missing operand");
    }

    fs::path p = ExpandHome(args[0]);
    if (p.is_relative()) p = fs::current_path() / p;

    try {
        fs::create_directories(p);
    } catch (const fs::filesystem_error& e) {
        throw RedTops::CommandError(std::string("mkdir: ") + e.what());
    }
}

// ---------- rm ----------
void RmCommand::Execute(const std::vector<std::string>& args) {
    if (args.empty()) {
        throw RedTops::CommandError("rm: missing operand");
    }

    bool recursive = false;
    std::vector<std::string> operands;

    for (auto &a : args) {
        if (a == "-r" || a == "-R") recursive = true;
        else operands.push_back(a);
    }

    if (operands.empty()) {
        throw RedTops::CommandError("rm: missing operand");
    }

    for (auto &op : operands) {
        fs::path p = ExpandHome(op);
        if (p.is_relative()) p = fs::current_path() / p;

        try {
            if (!fs::exists(p)) {
                throw RedTops::CommandError("rm: " + op + ": No such file or directory");
            }
            if (fs::is_directory(p) && !recursive) {
                throw RedTops::CommandError("rm: " + op + ": is a directory (use -r to remove directories)");
            }

            std::error_code ec;
            if (fs::is_directory(p))
                fs::remove_all(p, ec);
            else
                fs::remove(p, ec);

            if (ec) throw RedTops::CommandError("rm: " + ec.message());

        } catch (const fs::filesystem_error& e) {
            throw RedTops::CommandError(std::string("rm: ") + e.what());
        }
    }
}

// ---------- cp ----------
void CpCommand::Execute(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        throw RedTops::CommandError("cp: usage: cp [-r] <source> <dest>");
    }

    bool recursive = false;
    std::vector<std::string> ops;
    for (auto &a : args) {
        if (a == "-r" || a == "-R") recursive = true;
        else ops.push_back(a);
    }

    if (ops.size() != 2) {
        throw RedTops::CommandError("cp: usage: cp [-r] <source> <dest>");
    }

    fs::path src = ExpandHome(ops[0]);
    fs::path dst = ExpandHome(ops[1]);
    if (src.is_relative()) src = fs::current_path() / src;
    if (dst.is_relative()) dst = fs::current_path() / dst;

    try {
        if (!fs::exists(src)) {
            throw RedTops::CommandError("cp: source does not exist: " + src.string());
        }

        if (fs::is_directory(src) && !recursive) {
            throw RedTops::CommandError("cp: omitting directory (use -r to copy directories): " + src.string());
        }

        if (fs::is_directory(src)) {
            for (auto &entry : fs::recursive_directory_iterator(src)) {
                auto rel = fs::relative(entry.path(), src);
                fs::path target = dst / rel;
                if (entry.is_directory())
                    fs::create_directories(target);
                else {
                    fs::create_directories(target.parent_path());
                    fs::copy_file(entry.path(), target, fs::copy_options::overwrite_existing);
                }
            }
        } else {
            if (fs::is_directory(dst))
                dst /= src.filename();
            fs::create_directories(dst.parent_path());
            fs::copy_file(src, dst, fs::copy_options::overwrite_existing);
        }
    } catch (const fs::filesystem_error& e) {
        throw RedTops::CommandError(std::string("cp: ") + e.what());
    }
}

// ---------- mv ----------
void MvCommand::Execute(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        throw RedTops::CommandError("mv: usage: mv <source> <dest>");
    }

    fs::path src = ExpandHome(args[0]);
    fs::path dst = ExpandHome(args[1]);
    if (src.is_relative()) src = fs::current_path() / src;
    if (dst.is_relative()) dst = fs::current_path() / dst;

    try {
        if (!fs::exists(src)) {
            throw RedTops::CommandError("mv: source does not exist: " + src.string());
        }

        if (fs::exists(dst) && fs::is_directory(dst))
            dst /= src.filename();
        else
            fs::create_directories(dst.parent_path());

        fs::rename(src, dst);

    } catch (const fs::filesystem_error& e) {
        throw RedTops::CommandError(std::string("mv: ") + e.what());
    }
}

