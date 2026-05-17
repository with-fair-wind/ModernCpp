补充与总结 Supplementary and Summary

## 现代C++基础

## Modern C++ Basics

Jiaming Liang, undergraduate from Peking University

Postgraduate from PKU since 2024.9 :-)

### • File system

### • Time utilities (Chrono)

### • Math utilities

### • Summary and future prospect

# Supplementary and Summary

File system

# Overview

- Files represent organized data in non-volatile storage to let programs share data across different runs.
- Files are named collection of data.
- Directories are used to construct file hierarchy.
- Directories are named collection of files and directories.

- File system is an abstraction layer provided by OS to enable users to use path to access files and directories.
- It records metadata of files and directories (size, modification time, owner, etc.) to make them organized and hierarchical.
- C++17 includes related utilities in <filesystem>.

# Supplementary

- File system
- Path operations
- Overview
- std::filesystem::path
- File system operations

# Path Overview

- Essentially, path is a string that represents location of a file.
- There are two different kinds of paths:

1. Absolute path: always refer to the same location.
2. Relative path: the location relative to current working directory (CWD) for the current process.

- By changing CWD, the process can get different locations.
- A path consists of these components:

1. Root name (optional): like drive name in Windows (C:, D:); or UNC (//machine), etc.
2. Root directory (optional): a directory separator (\ on Windows, / on Linux, : on classic MacOS).
3. Relative path: a sequence of filenames separated by directory separator.

# Path Overview

- Besides no separators, filenames have many other platform- dependent characteristics or restrictions. e.g. on Windows:

e.g. well-known 260 in some Windows functions.

e.g. CON on Windows.

e.g. Linux is case-sensitive while Windows not.

Particularly, . and .. means current directory and parent directory respectively.

# Path Overview

- To make program cross-platform, C++ regulates a “generic format” that uses POSIX convention.
- That is, these three components are just concatenated to form a path.
- And / is considered as universal separator.
- Besides, C++ allows “native format” that depends on file system.
- E.g. OpenVMS, a legacy system previously used in bank (well, if it really supports C++17 utilities).

e.g. DKA0:[JDOE.DATA]test.txt

# Path Overview

- Note 1: “generic” only means it’s a valid format in all systems; but its location is not always the same.
- What is D:\sub\path?
- Windows: an absolute path at D drive, with two components sub and path.
- Linux: a relative path with name “D:\sub\path”, i.e. the whole string is a single component.
- What is /home/user?
- Linux: an absolute path with two components home and user.
- Windows: a relative path at drive of CWD, with two components home and user.
- Note 2: it’s not very safe to rely on relative path since it’s just like a global variable, which can be changed by other threads and external library.
- i.e. the location of a relative path can be modified arbitrarily.

# Path Overview

- Note 3: “relative” or “absolute” only means whether it’s interfered by CWD; there can be many paths that refer to the same location.
- E.g. /home/user, /home/user/., /home/user/dir/.., ….
- To unify all representations, we can do path normalization.
- There are two kinds of normalization:
- Lexical: a string-level substitution, which doesn’t change whether the path is relative or absolute.
- So /home/user, /home/user/., /home/user/dir/.. are all normalized to /home/user, and paths ./user, ./user/., ./user/dir/.. are all normalized to user.
- Filesystem-dependent: paths are normalized to a unique absolute path.
- Assuming CWD is /home, ./user, ./user/., ./user/dir/.. are all normalized to /home/user.
- C++ call such normalization as “canonical”.

# Path Overview

- Specifically, a normalized path requires:

POSIX may (Due to or may not UNC path) respect //.

Credit: C++17 the Complete Guide, Nicolai M. Josuttis.

# Supplementary

- File system
- Path operations
- Overview
- std::filesystem::path
- File system operations

For brevity, we use namespace stdfs = std::filesystem.

# Path

- C++ uses stdfs::path to represent a path.
- It is essentially a string of some native encoding, e.g. UTF-8 on Linux, \* UTF-16 on Windows.
- The underlying character can be checked by stdfs::path::value_type, normally char on Linux and wchar_t on Windows.
- And stdfs::path::string_type = std::basic_string<value_type>.
- And the string stores path in native format.
- You can just access the underlying string in const way:

\*: Strictly speaking, usually a filesystem doesn’t really respect encoding; it just treats the path as some byte sequence (even if it’s not a valid UTF-8 / 16). So “native encoding” essentially means “a string that you can pass into filesystem syscall directly”, e.g. 2-byte-per-unit sequence on Windows.

# Path

- However, you can construct the path by any encoding and format.

- For format, it’s essentially a scoped enumeration in stdfs::path with three enumerators:

- By default it uses auto_format, i.e. determine if the input format is native or generic automatically and convert if necessary.

# Path

- The template allows you to use any character type, and ctor will convert to its native encoding.

- For wchar_t, Windows won’t do any conversion but Linux needs to do so;
- For char, Linux won’t do any conversion but Windows needs to do so.
- Actually, Windows recognizes char for file API in ANSI (or Active) Code Page (ACP).
- This will lead to complex behavior when interacting with compiler option…

# Windows ACP

- Assuming that we have a file path “D:\试验.txt” on Windows.
- And we use a default Chinese PC, i.e. ACP is GBK (id: 936).
- Given main.cpp as:

Check whether some path exists, equiv. to std::ifstream{p}.is_open() if p is a file instead of directory.

- Case 1: msvc doesn’t add any option, and encoding of main.cpp is GBK.

1. D:\试验.txt is in GBK, and msvc reads it as GBK correctly.
2. The execution charset is GBK, so D:\试验.txt is still GBK in binary exe.
3. Current ACP is GBK, so stdfs::path converts it from GBK to native encoding (UTF-16) and stores it;
4. The path is correct so file system says it exists.

# Windows ACP

- Case 2: msvc adds /utf-8, and encoding of main.cpp is UTF-8.

1. D:\试验.txt is in UTF-8, and msvc reads it as UTF-8 correctly.
2. The execution charset is UTF-8, so D:\试验.txt is UTF-8 in binary exe.
3. Current ACP is GBK, so stdfs::path converts it from GBK to native encoding (UTF-16) and stores it;

- However, UTF-8 string is not really a GBK string, and the corresponding binary leads to GBK as “D:\璇曢獙.txt”.

4. The path is not correct and file system says it doesn’t exist.

- Case 3: msvc adds /source-charset:utf-8, and encoding of main.cpp is UTF-8.

1. D:\试验.txt is in UTF-8, and msvc reads it as UTF-8 correctly.
2. As default execution charset is ACP, D:\试验.txt is GBK in binary exe.
3. So the path is still correct and file system says it exists.

# Windows ACP

- Case 4: msvc adds /utf-8, and encoding of main.cpp is GBK.

1. D:\试验.txt in GBK is not valid UTF-8, so msvc warns C4828 (illegal character in UTF-8) and silently passes the original bytes as is.

- So accidentally, it’s still GBK in binary exe.

2. Thus accidentally, the path is correct and file system says it exists.

- Case 5: assuming ACP is UTF-8 (65001), msvc doesn’t add any option (equiv. to add /utf-8), and encoding of main.cpp is GBK.

1. Same as case 4, i.e. the string is of GBK in binary exe.
2. However, GBK is not valid UTF-8, so ctor of path throws an exception (std::system_error in MS-STL).

# Path construction

- So to make a valid path, we need first:
- Make sure the compiler knows how to read the file (string literals), i.e. the file encoding should be correctly specified in source charset.
- And then two ways:

1. Make execution charset (for string literals) and string encoding (for other strings stored in e.g. std::string) same as ACP.
2. Use char8_t[] / char16_t[] / char32_t[] instead. A. Any ACP is OK, since char8_t as a unique type will always be decoded as UTF-8 in ctor. B. Any execution charset is OK, since char8_t is always UTF-8 in binary exe.

- And if you know your std::string / … is essentially UTF-8 while ACP is not, you can reinterpret_cast it to avoid conversion.

# Path construction

- On the other hand, on Linux + gcc, no matter what execution charset you use, char[] won’t do any conversion.
- As char is its native encoding, so libstdc++ assumes correct bytes.
- Instead, -fwide-exec-charset will specify encoding of wchar_t and be converted to UTF-8 automatically.
- To specify encoding and conversion explicitly, you can use locale:

- As the conversion is explicitly specified in locale, the template only accepts a char sequence.

# Path construction

- When native encoding is wchar_t:
- Just use codecvt<wchar_t, char, std::mbstate_t> in locale to convert char[] to native encoding wchar_t[].
- E.g. For windows, GBK -> UTF-16.
- When native encoding is char:
- First use codecvt<wchar_t, char, std::mbstate_t> in locale to convert char[] to wchar_t[];
- E.g. For Linux, GBK -> UTF-32
- Then convert wchar_t back to native encoding char (e.g. UTF-8).
- E.g. UTF-32 -> UTF-8, which is equiv. to construct from wchar_t[] directly.

Default is just an empty string.

- And finally some omitted overloads & operator=, just list here.

# Path

- Besides construction, you can also get string of different formats and encodings.
- Native format + Native encoding: just .native(), as we mentioned.
- Native format + Converted encoding:

- Generic format + Converted encoding:

- Particularly, these functions are required to use / as directory separator.

# Path

- And there are also template versions, together will allocator:

- So to get Generic format + Native encoding, just use .generic_string<stdfs::path::value_type>().
- Finally, you can also check the preferred separator bystatic constexpr stdfs::path::preferred_separator.
- \ on Windows, / on Linux.

Note: filename with pure extension (e.g. "D:\\.gitignore") is not considered as extension (i.e. stem = filename =

# Path decomposition

".gitignore", extension = "")

- There are also some observer functions to query path : Concatenation components: Path: Appendage
- which just return new stdfs::path.

Root path Relative pathPath

RootRootParent pathFilename nameDirectory Stem Extension (Has dot)

# Path decomposition

- Particularly, such decomposition is lexical, which doesn’t even really interact with file system.
- So “parent path” doesn’t really return path of parent directory, but just remove the last component.
- And when a path ends with directory separator, the last component is just empty, so parent just removes the separator.

- For example:

Note: parent of root directory is still root directory (i.e. "/" -> "/"); but parent of file name is empty (i.e. "data.txt" -> "").

# Path normalization

- To find real parent, you need to do path normalization first.
- And we say that there are two ways:
- Lexical: by .lexically_normal(); the normalization process is: Assuming we have a path as “D:/..\sub\.\/path\..\file.txt”. D:/..\sub\.\/path\..\file.txt D:\..\sub\.\/path\..\file.txt

D:\..\sub\.\path\..\file.txt D:\..\sub\path\..\file.txt

D:\..\sub\file.txt D:\sub\file.txt 7. is applied to e.g. ..\..\.

1. & 8. An empty path is still empty after normalization, but a non-empty but essentially empty is normalized to ..

# Path normalization

- Filesystem-dependent: stdfs::canonical / weakly_canonical; paths are normalized to a unique absolute path.
- Path is first converted to an absolute path by stdfs::absolute(p);
- Then perform lexical normalization.

- canonical will check whether the path really exists, while weakly_canonical just normalizes it.
- We’ll go into details about two forms of global APIs in stdfs later.

Example

The last / is not stripped, even after normalization. .. only removes trailing /.

# Path normalization

- Notice that “physical” parent of lexical normalization may still be wrong when normalized result still contains ...
- Only (weakly\_)canonical ensures a correct physical parent.

- Finally, you can get or set CWD by current_path():

- So absolute() is essentially current_path() / path when path is relative.

cd: change current directory

“D:” : switch to current directory of Drive D.

# DOS directory

“C:” : switch to current directory of Drive C.

- It’s also worth nothing that DOS maintains separate “current directory” for every drive.
- So C: and D: are actually relative paths, while C:\ and D:\ are absolute paths.
- In Windows, current directory is unified as a single path, as known by CWD.
- However, CMD pretends they are still there by storing them with “strange environment variables”.
- And Windows inherits the DOS behavior, regarding C: and D: as relative paths. But there exists only a single real CWD.

Other non-CWD drive will return root.

1: term “relativization” is from Java.nio.

# Path relativization

- In contrast to normalization, path can also be “denormalized” by converting an absolute path to relative path.
- More generally, given a path b, how can it be transformed to path a with shortest components.
- For example, path{ "/a/d" }.relative("/a/b/c") would be "../../d".
- Similarly, you can use two ways:
- Lexical: by a.lexically_relative(b); the process is: ① Check whether it’s possible to transform b to a.
- If it’s impossible (i.e. conditions below), return empty path directly.

E.g. two paths in different drives in Windows. Absolute path + relative path, not lexically transformable. E.g. bar and /foo on Windows, i.e. you cannot cd from /foo to bar without CWD.

E.g. for UNC path on Windows, e.g. \\.\C:\Test uses C:\Test as relative path. That is, UNC doesn’t participate in lexical relative process.

a = D:\test\test.txt b = D:\test\test2\test.txt\..

# Path relativization

② Determine the first mismatched component of two paths (just like std::mismatch).

- Let remaining mismatched components of a be [𝑎1, 𝑎2) and b be [𝑏1, 𝑏2);
- Example above is 𝑎, 𝑎= 𝑡𝑒𝑠𝑡. 𝑡𝑥𝑡 , 𝑏, 𝑏= [𝑡𝑒𝑠𝑡2, 𝑡𝑒𝑠𝑡. 𝑡𝑥𝑡, . . ]. 12 12
- If no mismatched component (i.e. 𝑎1= 𝑎2, 𝑏1= 𝑏2), return path{ “.” };
- Otherwise, assuming that in [𝑏1, 𝑏2), 𝑛 components are .. and 𝑚 components are not .., . and empty.
- Example above is 𝑛 = 1, 𝑚 = 2.
- If 𝑛 > 𝑚 (that is, the lexically normalized form contains only ..), then return empty path.
- If 𝑛 = 𝑚 (that is, the lexically normalized form is .), then:
- If 𝑎= 𝑎, return path{ “.” }; 1 2
- Otherwise, return [𝑎, 𝑎).
- If 𝑛 < 𝑚, then return a path with 1. ".." repeated for 𝑚 − 𝑛 times; 2. [𝑎, 𝑎). result = ..\test.txt

# Path relativization

- Actually, this algorithm may falsely report empty path even when such transformation should be possible. 𝑛 = 1, 𝑚 = 0
- For example:

- Though theoretically it can be “..”.
- If you want an always-correct lexical transformation, you need to do lexical normalization first.

- The second way is filesystem-dependent stdfs::relative(a, b), which will always ensure correct relative path in the file system.
- It’s same as lexically_relative two weakly_canonical paths.

# Path proximation

- Finally there also exists proximation, which means “relativization if possible, otherwise return original path”.
- Effectively:

- And stdfs::proximate(a, b) is also same as lexically_proximate two weakly_canonical paths.
- BTW, when b is not provided in relative and proximate, current_directory will be used.

# Path composition

- For a given path ./sub/path/file.txt, it’s essentially a combination of hierarchical components.
- C++ provides two utilities to combine components.

1. Append: combine two components with a directory separator, if needed.

- For example: (on Linux)

- However, there exist lots of corner cases… ① Subpath is absolute path: C++ chooses to overwrite (replace) LHS.
- For example:

- But this can be astonishing:

Reason: “/.fonts” is absolute path.

# Path composition

- DOS-like behavior on Windows also causes surprising result even when subpath is relative. ② No separator is inserted for a single drive; it’s still a relative path.

③ Appending a relative path that has different drive will replace the whole path；

④ Appending a relative path that has same drive will append as if LHS is CWD.

⑤ Appending a relative path that has root directory but no drive, to another path with drive will reserve LHS drive.

# Path composition

2. Concatenate: combine two components as if concatenating underlying strings directly; no additional separator is introduced.

These overloads are designed to mimic overloads of std::string ::operator+=.

- Strangely, there is no operator+; but normally this operation is used to concatenate with a string, so you can just operator+ all strings first.
- Or you have to use either ((stdfs::path{a} += b) += c)… or .native() to use operator+ of std::basic_string.

# Path composition

- Note 1: Due to associativity, p / "a" / "b" is legal while p /= "a" /= "b" is illegal.
- p / "a" / "b"  ((p / "a") / "b"), while
- p /= "a" /= "b"  (p /= ("a" /= "b")).
- And it’s illegal for two string literals to /=.
- You have to add a bunch of brackets; that’s why we use ((stdfs::path{a} += b) += c)….
- Note 2: there also exist some boolean observers to check existence.
- “empty” means the underlying string contains nothing.
- And has_xxx means whether xxx is empty or not.

# Path iteration

- As a combination of many different components, path can also be iterated (grouped by separator) in generic format.
- It provides .begin() and .end() that return a path const iterator.
- It just iterates through root name, root directory and filenames.
- For example:

Deferenced result (i.e. iterator::value_type) is another path.

# Path iteration

- Though it seems to be bidirectional iterator, it’s actually only regulated to be input iterator.
- Reason: before C++20, forward iterator has such a regulation:

- That is, every component should have a fixed source to make every iterator dereferenced to that source.
- This requires path to store a container of components, making it expensive to construct any path…
- And this is how libstdc++ implements it, making it bidirectional.
- However, path iterator is quite like a string std::views::split by separator!
- Another way (as libc++ and MS-STL do) is to cache the range in the iterator, so only when iterator is used will parsing begins.

Issue 2674: Bidirectional iterator requirement on path::iterator is very expensive

# Path iteration

- So instead, the standard regulates that:

- which makes it only satisfies input iterator, and thus it’s impossible to apply some functions in <algorithm>.

- BUT, such requirement is not part of bidirectional_iterator in C++20!
- So theoretically it should be able to utilize constrained algorithms, i.e. std::ranges::xxx.
- Well, this problem is much more complicated… See our homework for discussion.
- Anyway, libc++ already adds iterator_concept for it, while MS-STL cannot do so.

# Path modificationReturn reference

to \*this.

- There also exist some simple non-const methods:

1. .make_preferred(): for path whose native format is also generic format, convert current separators to preferred separators.

- For example:

2. .remove_filename(): Remove the last component (if it exists) so .has_filename() returns false.

- So after removal, the path is either empty or ends with a separator.

“foo” will also be converted to “”.

# Path modification

3. .replace*filename(const path& rep): equivalent to 1. this->remove* filename(); 2. (\*this) /= rep.
4. .replace_extension(const path& rep = {}): equivalent to code below:

- For example:

# PathBefore element-wise comparison, it

needs to judge these conditions first.

- And finally some simple utilities, just list here.e.g."D:/Test"
- Comparable (by <=>/== or .compare); compare components.== "D://Test"
- Hashable (by std::hash or friend hash_value); hash components.
- Input / Output by >> / <<;
- For i/ostream<CharT, Traits>, equiv. to input / output the .string <CharT, Traits>() with std::quoted so space will not interrupt input.
- Recap: quoted will escape the quote and the escape, so \ is escaped to \\.
- Formattable since C++26.

# Path formatting

- Its regulation is slightly different from .string().

i.e. encoding of chari.e. explicitly regulate that in UTF-8 string literal, as specifiedexecution charset on Windows, illegal in compiler executioncharacter will be converted to U+FFFD. charset option.

As a C++26 feature, it’s not yet implemented so “implementation-defined” is unknown (but likely to be printable with std::print).

# Final Notes

- Note 1: there also exists .u8path() to construct from a UTF-8 string in C++17, which is then deprecated in C++20.
- Reason: C++20 introduces char8_t, which distinguishes UTF-8 sequence from char by template so there is no need to introduce a new method.
- For the same reason, .(generic\_)u8string returns std::string in C++17 and std::u8string in C++20.
- Note 2: to use path as key in map, you usually need to normalize it first by canonical.
- Reason: comparison and hashing of path are performed lexically for the underlying components.
- Without normalization, two equivalent paths may be seen as two keys.
- On Windows, you may even need to to_lower all case-insensitive paths.
- A more expensive but always-correct way is by stdfs::equivalent to compare, which needs file system call to check equality of two paths. Covered later.

# Supplementary

- File system
- Path operations
- File system operations
- Overview
- File status query and directory iteration
- Modification operations

# Overview

- In this section, most of the functions interact with the underlying filesystem, which are in the global scope stdfs::.
- By contrast, functions we taught in the last section are purely lexical (and thus cheaper), which are member functions of stdfs::path.
- Almost every function provides two versions:

1. Error code version: add std::error_code& as the last parameter to return filesystem error (noexcept if only filesystem error is possible);
2. Exception version: throw stdfs::filesystem_error to represent error.

- Reason: filesystem operations can easily incur TOC/TOU problem, so pre- check cannot prevent error.
- Thus, it’s hard to predict whether error occurrence is in hot path or not, making exceptions sometimes not proper.

# Overview

- For example, if we want to write a “chmod when it exists”.
- So pseudocode can be:

- But filesystem is cross-process, so the events can be:
- We check that file indeed exists (TOC);
- Another process removes this file;
- We change permission of the file, which doesn’t exist and causes error (TOU).
- When multiple processes access the same filesystem object (i.e. race), the specific behavior is implementation-defined.

- Even worse, attackers can easily leverage TOC/TOU.
- A well-known one is stdfs::remove_all (also in Rust!), which removes all files under the directory recursively but skip removing files in “symbolic links”.
- We’ll cover symbolic links later; but generally it means an object that refers to another directory.
- Such skip can prevent users from accidentally removing files in other folders.
- And all three standard libraries initially implement it as:

1. Check whether the object is a symbolic link;
2. If it is not, recursively remove its files.

- Say hackers want to remove sensitive/ but they don’t have permission; but current system runs a privileged program with stdfs::remove_all, which periodically removes recyclebin/ that don’t add permission.

1. Hackers create a directory called temp in recyclebin/ first;
2. remove_all checks that it’s not a symbolic link;
3. Hackers delete temp and create a symbolic link temp to sensitive/.
4. remove_all removes all files in temp, i.e. all files in sensitive. Hackers win!

# Overview

- Similarly, user code can become vulnerable for TOC/TOU…
- Core problem: “path name” is a mutable property; a more robust way to always refer to the same filesystem object should be some handle.
- Which is proposed in P1883 (i.e. the low-level file i/o library);
- However, this can make filesystem APIs very obscure to use. -------- Back to standard ---------
- APIs of stdfs::filesystem_error are quite simple:
- We don’t dig into std::error_code here; see our homework for details.

# Supplementary

- File system
- Path operations
- File system operations
- Overview
- File status query and directory iteration
- Modification operations

# File status

- A file has the following attributes:
- Name, as used in path;
- Type;C++ uses POSIX conventions, but allows slight
- Permission;customization for different systems (e.g. Windows).
- Size;
- Last modification time.
- And POSIX regulates the following file types (as reflected in stdfs::file_type):
- Implementations are allowed to add new types (e.g. junction in MS-STL).

# File types

What are these types?

# Link

- There are many kinds of links in the filesystem.
- Essentially, links are just objects that redirect to other objects;
- Different links are just redirection at different levels.
- A simple illusion of how filesystem handles objects:

/home/dir1/file.txt /home/dir1 /home/home/dir2/file1.txtFilesystem operates the tree. /home/dir2 As if ashared_ptr;/home/dir2/file2.txt when all pointers to the node are erased, the node will also be… Nodes of filesystem objects released.

# Hard Link

1. Hard link: as if adding reference count to underlying data nodes.

- Filesystem cannot distinguish a hard link with the original object (so there is no type called “hard link”!).
- When you delete /home/dir2/file2.txt, the content isn’t really deleted; /home/dir1/hardlink.txt still keeps the node.
- Hard link count of an object can be checked by stdfs::hard_link_count(p). /home/dir1/hardlink.txt

/home/dir1/file.txt /home/dir1 /home/home/dir2/file1.txtAdd hard link to /home/dir2/home/dir2/file2.txt; /home/dir2/file2.txt

…

# Hard Link

- Most of the filesystems don’t allow users to create hard link to directories easily.
- Core reason: filesystem is usually assumed to form a tree.
- However, creating hard link of some directory in its descendent will make a cycle in the graph, making it not a tree.
- Some system programs may traverse the filesystem without special check; a circle will cause infinite loop.
- Linux: ln hd.txt a.txt; do not allow hard link to directory.
- MacOS: ln hd.txt a.txt; no native command for directory but can use POSIX link();
- MacOS link implementation checks whether new hard link to directory causes cycle; allow if not (but possibly need root privilege).
- Windows: mklink /H hd.txt a.txt; do not allow hard link to directory.

# Hard Link

- Note 1: hard_link_count for directory is implementation-defined.
- Unix: “.” and “..” are all seen as hard link; so a new directory will have hard_link_count as 2 and ++hard_link_count of its parent directory.
- Windows: always return 1.
- Return type: uintmax_t.
- Note 2: hard link has some other restrictions:

1. Some filesystems don’t support hard link (notably FAT file system; most of USB flash drives (U盘) use it).
2. Hard link is not cross-filesystem since different filesystems may have different data structures. Hard link exists only in the same filesystem.

- Particularly on Windows, they must be in the same volume (notably drive).

3. Some filesystems may have limits for hard links per file.

# Symbolic Link

2. Symbolic link (soft link): as if pointing to filesystem entry.

- Or, you can think it as a weak_ptr to the underlying node.
- When the real entry /home/dir2/file2.txt is deleted, its content will also be deleted though symbolic link /home/dir1/softlink.txt exists.
- Any redirection operation of the soft link later (e.g. file I/O) will fail.

/home/dir1/softlink.txt

/home/dir1/file.txt Add symbolic link to /home/dir1 /home/dir2/file2.txt; /home/home/dir2/file1.txt /home/dir2 /home/dir2/file2.txt

…

Filesystem functionality can be checked in MS Doc. Notice that UDF is widely used in optical discs (光盘).

# Symbolic Link

- Filesystem treats it as a distinct file type, but most native APIs will automatically redirect it to the real entry.
- That is, users (instead of filesystem!) cannot distinguish symbolic link with a normal object, unless they use a few special functions.
- Therefore, filesystem allows symbolic link to directory.
- Unix: ln –s sym a;
- Windows: mklink /D sym a; Particularly, creating symbolic link on Windows requires root privilege.

- Note: Some filesystems don’t support soft link (notably FAT).
- But as it points to filesystem entry, it’s cross-filesystem.
- Therefore, it’s okay to create soft link to FAT on NTFS since NTFS supports it (but not vice versa).

# Junction

3. For non-root privilege, Windows allows link to directory by a new type called “junction”.

- mklink /J junc a.
- Like symbolic link, when the real node is deleted, junction still exists but becomes invalid.
- Though junction is cross-filesystem, it only allows to link directory in local computer.
- i.e. no support for network path and UNC.

- Most of stdfs APIs will follow all links above and resolve to the final real entry.
- For example, stdfs::exists; stdfs::absolute; and thus stdfs::canonical; stdfs::relative; stdfs::proximate.

Check this blog for link details on Windows.

# Shortcut

- BTW, there also exists “shortcut” (快捷方式) on Windows, which is actually a “user-space symbolic link”.
- It’s a file with “.lnk” extension; filesystem just treats it as a normal file (and it is!).
- Its content is the real path of the filesystem object, how it should be launched, etc.
- So guess: when you click shortcut, how are you redirected to the real entry?
- Yes, Windows Explorer (文件资源管理器) UI helps you!
- The Explorer program automatically redirects your click as if you click the real entry; other programs that don’t do special process cannot redirect.
- By contrast, links will be redirected by filesystem automatically even if programs don’t do special process.

# Character and Block File

- When you plug in a USB (mouse, disk, etc.), how can OS identify the way to interact with the connected device?
- By drive program, if you’ve learnt Embedded System.
- When you want to support a new external device, you can write your own drive with a bunch of functions (e.g. open, write, etc.) to make OS know how to interact with it.
- And finally, your drive needs to expose a device file to make users able to open, write, etc. by OS APIs.
- Character file and block file are just such device files.
- When you read from / write to them, it’s as if you get data from / input data to your drive and thus successfully interact with your device.
- Character file means “I/O is passed directly without buffering”;e.g. terminal: /dev/tty
- Block file means “Buffering I/O and pass until proper time”.e.g. SSD: /dev/sda

Windows also supports character file (e.g. CON) but MS-STL always returns false for these two types since it’s expensive to detect such file type.

# FIFO

- FIFO is just named pipe.
- In Linux, we can pass output of a program as input to another program by | (e.g. cat data.txt | grep "info").
- Pipe allows the data to keep in memory instead of writing into real files (and thus real storage) for inter-process communication.
- Sometimes, it’s inconvenient to create anonymous pipes and pass them to other programs.
- Instead, you can create a named pipe in the filesystem, with some programs writing and other programs reading it (again, happen in memory without really going down to real storage).
- E.g. in command line: mkfifo test; cat data.txt > test &; grep "info" test; or by mkfifo in POSIX API.
- So test just has fifo type, which is specially interpreted in POSIX system.

Fill network location.

# Socket

- Similarly, sometimes it’s inconvenient to use network socket to communicate across processes.
- Instead, you can create a socket file (a.k.a. Unix domain socket, UDS).
- We know how to create a network server socket in Linux.
- For UDS, just change flag and location to local socket file:

After bind, process will create sockfile.sock in CWD, which has file type as socket.

- However, bind requires the address to be not already used, so the socket file shouldn’t exist before bind (thus, it’s usually a temporary file).

# FIFO and Socket

- Note 1: difference between fifo and socket:

1. Socket file can use send or recv, as network socket does; fifo can only use write or read.
2. Socket can use datagram instead of byte stream (by SOCK_DGRAM), while fifo only uses byte stream.
3. Socket is bidirectional, i.e. server and client can send or recv the other freely; fifo is unidirectional, i.e. sender always send and receiver always recv.
4. Typically, fifo is used for two users, while socket can be used for multiple clients.

- Note 2: Windows also supports fifo (since Windows 2000 professional) and UDS (since Windows 10 17063 (2017/12)).
- However, MS-STL always returns false for is_fifo and is_socket.

Reason: UDS doesn’t create a real file like in UNIX.

# Permission

- For permission, there are three levels in POSIX: user / group / all.
- Each level can have read / write / execution right.
- In Linux bash, we can change permission by chmod; e.g. chmod 764 means that owner can read, write or execute, current group can read and write, while all others can only read.
- As each level can be expressed in three bits, you can use octal number to form the permission in C++.
- E.g. literal 0764, since leading 0 means octal literal.
- However, Windows permission system (Active Control List, ACL) is not compatible with POSIX. It thus uses naïve DOS permission:

1. All users can read, write and execute the file (777);
2. All users can read and execute the file (555).

# Permission

- In POSIX (again, not in Windows), there also exist three special permissions: setuid, setgid and sticky bit.
- setuid: when executing the binary, use the owner’s user id (so that it can execute code that only owner can execute).
- Typical example: /bin/passwd, which will write /etc/shadow. Users can change password by /bin/passwd, but don’t have privilege to write /etc/shadow (otherwise they can change anyone’s password!).
- Thus, /bin/passwd has permission setuid, i.e. it has root privilege to write /etc/shadow when executing /bin/passwd.
- Since /bin/passwd controls what it accesses, it’s still safe.
- setgid: when executing the binary, use the owner’s group id.
- Typical example: /usr/bin/wall, which needs group privilege to write to others’ tty.

# Permission

- sticky bit: typically means “when applying to a directory, only file owner can delete his/her file even if he/she can write the directory”.
- Typical example: /tmp; all users have write permission to /tmp to keep temporary files. They may delete their cache files later.
- However, this then allows the user to delete others’ file, which may randomly crash others’ applications.
- Solution 1: every user protects his/her file in /tmp by e.g. chmod 007.
- Well, that’s too troublesome…
- Solution 2: apply sticky bit to /tmp!
- Then only file owner can delete his/her file.

- C++ defines these permission specifications with enumeration.

# Permission

- Permission values are defined as enum class perms with these enumerators:
- With overloaded bit operators.

# File status query

- To test file type, you can use these APIs:
- Params: (const path& p[, error_code]).

Other file: file exists but isn’t regular file / directory / symlink.

# File status query

- However, a more efficient way is to query type once and cache it, so every test will be a cheap and plain integer comparison.
- Just by stdfs::status!
- For example:

Credit: C++17 the Complete Guide, Nicolai M. Josuttis.

# File status query

- status essentially returns file_status, with .type()->file_type and .permissions()->perms:
- Another example:

Note: is_xxx (e.g. is_directory) can also accept file_status, which is noexcept since it’s just plain integer comparison.

# File status query

- However, when you try to apply status to symbolic links:

- Oops, the type is not symlink, but directory!
- Reason: filesystem will automatically follow symbolic links in its APIs.
- To query the shallow status instead of following links, you should use symlink_status:

Note 1: is_symlink(p[, ec]) internally uses symlink_status. Note 2, again: hard link cannot be distinguished.

Here we create Dir2 as junction, which is listed as a new enumerator in MS-STL so is_symlink is still false (well, though it can be queried by symlink_status).

MS-STL: return 0; libc++ & libstdc++: throw error (“is a directory”);

# File status query

- The last two status are size and last modification time, which can also be queried by path:
- stdfs::file_size(p[, ec]) -> std::uintmax_t;
- The result of directory is implementation-defined.
- stdfs::last_write_time(p[, ec]) -> stdfs::file_time_type;
- The specific utilities in std::chrono will be introduced later.
- Note: except for type, you can change any status directly.
- stdfs::resize_file(p, newSize[, ec]);
- If newSize is greater than current size, fill new space with 0; otherwise truncate the file.
- Note: on filesystems that support sparse files, greater size of file may not decrease available storage in filesystem as long as “new space with 0” is not really written.

# File status query

- stdfs::last_write_time(p, newTime[, ec]);
- But the grain of filesystem may be not same as program time, so there can be rounding error.
- stdfs::permissions(p, newPerm[, opt[, ec]]);
- opt is specified as enum class stdfs::perm_options, also with bit operators.

- Default opt is replace.

# Directory entry

- Umm, so is there a way to query all attributes once and just get them from cache?
- By stdfs::directory_entry! Construct from path,
- For example: and ctor will query all attributes if they exist.

Note: in libstdc++, you need to #include<chrono> to make time outputable (C++20, covered later).

# Directory entry

- It has almost all query methods we covered before:
- And some utilities (only applied to the underlying path!):
- Comparable
- operator<<

.is_symlink uses symlink_status while others use status. So .is_symlink && .is_directory can be true.

# Directory entry

- And also some simple modifiers:

i.e. path.replace_filename(…) and .refresh(). i.e. Re-query attributes by filesystem calls.

- Applying these methods doesn’t e.g. rename the underlying file in the filesystem, but just re-query!
- Actually, directory_entry is value type of directory_iterator, which can be used to iterate over a path.

1. Besides operator++, the iterator also defines .increment(ec) to accept error code.

# Directory iteration2. Essentially, begin returns its own copy and

end returns default constructed iterator.

- It has friend begin and friend end and naturally forms a range, making it iterable directly: Path will include root (passed in ctor) as prefix.

- You can also add stdfs::directory_options as the second param in ctor to control iterator behavior: Again, it’s bit-operatable scoped enumeration.

# Directory iteration

- To iterate recursively in the directory, you can also use recursive_directory_iterator.
- which will then use implementation-defined way (usually DFS) to traverse all entries.
- For example:

Dir2 is junction to Dir1, and we follow symlink, so essentially contents in Dir1 is printed again.

# Directory iteration

- The iterator has more member methods:

->directory_options Observer: ->int (Starting from 0)

->bool

Modifier:

pop([ec]): discard following iterations in subtree and go to next entry in the last level. This operation will invalidate all previous copies; and when .depth() == 0, iterator will become end(). disable_recursion_pending: when the current entry is directory, do not go into the directory for next ++. recursion_pending will return false then. Increment will reset recursion_pending to true.

# Directory iteration

pop

depth == 0

disable\_ recursion \_pendingdepth == 1 then ++.

depth == 2

# Directory iteration

- Note 1: their operator\* returns const&, so you cannot modify directory_entry without copying it.
- Note 2: they are input iterators.
- Recap: input iterators are not forward iterators because they’re one-pass; copying it and iterating the copy may lead to different results.
- Here similarly, directory contents may be altered by other processes, so it’s not multi-pass.
- It’s actually more complicated and see our homework for detailed discussion.

- Note 3: if new files and directories are added in directory A after creation of (recursive\_)directory_iterator{ "A" }, it’s unspecified whether they will be iterated or not.

# Final Notes

- There exist some other minor read-only methods:

1. Path operations:

- current_path() -> p for CWD;
- temp_directory_path() -> p (e.g. /tmp on Linux);
- equivalent(p1, p2) -> bool for judging whether two paths are essentially same (following symlinks).
- read_symlink(p1) -> p2 for converting a symlink path to its target path (error if p1 is not symlink).

2. Filesystem operations:

- is_empty(p) -> bool, return true if p is empty file or directory;
- space(p) -> space_info, which determines space of mounted filesystem that p lies in.
- On Linux, you can check filesystem of the path by df –h:
- On Windows, each drive is seen as a mounted filesystem.

# Final Notes

- An example program from cppreference:

# Supplementary

- File system
- Path operations
- File system operations
- Overview
- File status query and directory iteration
- Modification operations

# Creation

- There are four kinds of modification operations:

1. Create:

① directory:

- create_directory(p[, existing_p[, ec]]) -> bool;
- When given existing_p, created p will copy OS-dependent attributes from another directory existing_p (Windows does nothing). Parent of p must exist (i.e. create only a single directory).
- create_directories(p[, ec]) -> bool;
- Create every element that doesn’t exist in directory path. So when parent of p doesn’t exist, it will be created.

return false if directory already exists (not seen as error) or an error occurs.

ATTENTION: what the path symlink refers to is from the view of symlink itself. (but hard link doesn’t do so).

# Creation

② links:

- create_hard_link(original_path, link_path[, ec]);
- create_symlink(original_path, link_path[, ec]);
- create_directory_symlink(original_path, link_path[, ec]);
- Some OS may need distinct APIs to create symbolic links to regular files and directories, so it’s recommended to use create_directory_symlink for directory. POSIX doesn’t distinguish them.
- Note: these two methods just create soft links, which then require root privilege in Windows. And, there is no way to create junction in Windows by standard library. ③ Regular files: not in stdfs; you can use std::ofstream to do so. Example:

# Copy

2. Copy:

- Copy supports copy_options (again, bit-operatable scoped enumeration) to configure copy behavior.
- copy(from, to[, opt[, ec]]): copy according to opt.
- copy_file(from, to[, opt[, ec]]): copy regular file according to opt.
- This can be cheaper than copy for less check, and doesn’t incur TOC/TOU compared with if(!is_directory(a)) copy(a, b).
- Error conditions (after following symbolic links, don’t cover OS error like permission):
- !is_regular_file(from); or
- exists(to) && (equivalent(from, to) || !is_regular_file(to) || opt == copy_options::none).

The specific behavior (including error conditions) for copy is too long and thus not covered here. Check details in cppreference.

# Copy option enumerators

(Also applied to copy for files).

# Copy

- copy_symlink(from, to[, ec]): copy symbolic link to another position, instead of following link and copy content like copy.
- Equiv. to create*(directory*)symlink(read_symlink(from), to).
- Note: copy_symlink doesn’t copy junction on Windows; you can only use copy(from, to, copy_options::copy_symlinks) to copy it.
- Example of copy:

# Remove and Rename

3. Remove:

- remove(p[, ec]) -> bool: requires p to be regular file or empty directory, symlink is not followed; return true if p is removed.
- remove_all(p[, ec]) -> uintmax_t: remove all contents in p (including p), symlink is not followed; return number of removed entries (-1 on error).

4. Rename:

- rename(old_p, new_p[, ec]):
- If new_p is hard link to old_p, nothing happens;
- If old_p is regular file: remove new_p if it exists and is a regular file, and rename to new_p.
- If old_p is directory: remove new_p if it exists and is a directory, and rename to new_p. Parent of new_p should exist and new_p shouldn’t end with separator.
- Note: other processes cannot observe such removal.

# Supplementary and Summary

Chrono

All utilities are defined in std::chrono; for brevity, we use namespace stdc = std::chrono.

# Overview

- We know most of physical quantities need units.
- 1 minutes = 60 seconds; 1 second = 1000 milliseconds; etc.
- And their ratio can be conveyed by rational numbers (有理数).
- In <chrono> library, such ratio is used for time units too.

intXX: Signed integer with at least XX bits.

# Supplementary

- Chrono
- Compile-time rational number
- Time
- Date and time zone

num: numerator, 分子 den: denominator, 分母

# Rational number

- We know rational number means a real number that can be expressed as ratio of two integers.
- Their computation is exact, i.e. doesn’t incur rounding error.
- So is it enough to just use struct R { int num; int den; }?
- Not really…

1. 3 / 2, 6 / 4, … are essentially same, i.e. it needs to do reduction.
2. class NTTP is introduced in C++20, while <ratio> is in C++11.

- Instead, it introduces a new type std::ratio<Num, Den>.
- which defines quantities after reduction:

e.g. std::ratio<6, 4> has num = 3, den = 2, type = std::ratio<3, 2>.

- Operations are defined on types:

These operations will reduce the computed type automatically (so no need to use ::type explicitly).

Alias for SI units:

# Supplementary

- Chrono
- Compile-time rational number
- Time
- Duration
- Time point
- Date and time zone

To use these literals, using namespace std::literals or

# Durationstd::chrono_literals.

- In physics, we define time point (时刻) and time (时间).
- Time is just interval between two time points.
- In C++, such interval is represented by stdc::duration:

Rep: how to represent the number, e.g. long long or double. Period: number unit (w.r.t. second), e.g. std::milli (std::ratio<1, 1000>) for milliseconds. Default means second.

- It essentially only stores a number of type Rep.
- You can get it (in value type instead of reference) by .count().
- It also provides some user-defined literals whose Period are adjusted accordingly:

auto is stdc::milliseconds, i.e. stdc::duration<intXX, std::milli>. Guess what about auto m = 10.0ms?

# Duration

- You can also convert between different representations & periods by stdc::duration_cast:

- For example:

- To keep numeric accuracy, it will find “largest” type for both types first.
- Representation of ToDuration is converted finally, i.e. after all computations are done.

\*: after decay, if no specialization.

# Common type

-

* Basically, std::common_type<A, B> is just type of conditional operator (i.e. cond ? std::declval<A>() : std::declval<B>()).
* The detailed procedures are quite complex; generally speaking, it chooses either A or B that can be implicitly converted from another.
* If none of or both of conversions can happen, then compilation error.
* But for arithmetic types, since all types can do implicit conversion, it checks usual arithmetic conversions.
* i.e. Floating-point > integers; more bits > less bits.
* std::common_type<T0, T1, …> just finds common type iteratively.
* i.e. std::common_type<std::common_type<T0, T1>, …>.
* So for duration_cast between two integers, intmax_t will be used during computation.

# Duration

- For ctor:

i.e. Float cannot be used to construct stdc::duration<Int> (so conversion is lossless).

i.e. either the duration uses floating-point, or Period2 is exactly divisible by Period (so conversion is lossless).

# Duration

- It also defines many arithmetic operations.
- Note 1: operator++/-- can only be used when the underlying representation can do so.
- Note 2: Pay attention to implicit conversion.
- For example:

- Output:
- Reason: these types are integers!
- So 1.5f is implicitly converted to integer 1.
- A similar one: auto m = 10ms; m \*= 1.5f;
- Solution: make representation a floating-point number.
- E.g. auto m = 10.0ms instead, or specify representation explicitly.

# Duration

- Note 3: other operators use common type of two durations.
- which is specialized to be common type of underlying representation.

# Duration

- Note 4: comparable:
- which will also convert to their common type first and then compare .count().

- Note 5: C++17 adds some simple math functions:
- For example:

# Duration

- C++20 adds more utilities:

1. Days, weeks, months and years: since every month and year have different length, it uses the average number.

- Accurate: 1 days = 86400s, 1 weeks = 7 days;
- Average: 1 years = 365.2425 days, 1 months = years.
- And also their integer aliases (after rounding):

- Note: they do NOT have literals; y and d are used to convey a year and a day, which is date (time point) instead of duration.
- Duration is years and days, not year and day.

BTW: since months isn’t multiple of days, days + months doesn’t result in a duration with unit days; the common type has a unit of 54 seconds. days + years is 216 seconds.

# Duration

2. I/O and formatter: ① operator<<: output .count() with additional unit, depending on Period type.

- For non-special types:

② formatter: default format is equiv. to operator<<.

- The specific formatting specifiers chrono-spec are same as strftime in C and time locale.
- It’s rather complicated so we just give a brief introduction; check manual for detailed usage.
- A simple example first:

Historical reason: convenient for shell input.

%O?: locale’s alternative numeric symbols.

%E?: locale’s alternative era-based representation (纪元/年号纪年法) .

# Duration format

- Note 1: the “precision” in seconds is not precision in format spec.
- As example above shows, 0.5 second is not output.
- But if we change it to milliseconds…
- Wow, it has three fractional digits now.
- So assuming that the period is 𝑃; 𝑀
- If there exist integers 𝐾, 𝑀 that satisfy 𝑃 = 𝐾 × 10, then precision is defined as max(−𝑀, 0).
- For example, 1 = 1 × 10, so precision is 0 and thus floating-point is output as integer. −3
- 1/1000 = 1 × 10, so precision is 3.
- Otherwise, precision is 6.
- For example, 1/3 cannot be expressed in form above and uses 6.

We’ll cover details of stdc::hh_mm_ss later.

# Duration format

- If you want a more sophisticated format, you can use stdc::hh_mm_ss.
- It’s just a utility class that equivalently converts duration to hour-minute- second integers, with subsecond for fractions.
- For example:

- It is also essentially where precision is defined:

# Duration format

- Note 2: currently effects of precision spec. in formatter are not clear for chrono (e.g. {:.3%Q} is valid but .3 has no effect).
- After asking proposal author of chrono library, I think this is a defect and may be solved by DR later.
- Note 3: 12-hour representation example (Windows):

Recap: L is needed for Print functions don’t acceptformat to use locale. locale (you need to change global locale), so we use format explicitly.

Some locale may not respect %r (platform- dependent).

Windows locales generally don’t handle %O and %E.

# Duration

- To ensure 12-hour representation, C++20 also adds some functions for judging and conversion.
- Result of out-of-bound time for makeXX is unspecified. ℎ ∈ [0, 11] ℎ ∈ [12, 23] 𝑟𝑒𝑠𝑢𝑙𝑡 ∈ [1, 12] (0 is 12 a.m.)

③ from_stream: scan a string with a format to get a time.

- i.e. inversion of formatting, quite like std::scan (scan is not yet accepted).

The last two parameters are for time zone and will be covered later.

Except for %Q/%q, it just accepts same chrono spec as format.

# Duration input

- For example:

- Failed read will set stream failbit without modifying time passed in.
- A manipulator stdc::parse is also introduced for help:

# Duration

3. Details of stdc::hh_mm_ss<Duration>:

Convert back to duration

As if “{:L%T}” for equivalent duration.

As if formatter for the equivalent duration; default format is still same as operator<< (i.e. as if “{:L%T}”).

# Duration

- Finally some rarely-used notes:
- Note 1: You can specialize stdc::treating_as_floating_point and stdc::duration_values for underlying representation.
- treating_as_floating_point: as its name.
- duration_values: need to define three functions.
- which will be exposed as static functions of stdc::duration too.
- Default: Rep(0); std::numeric_limits<Rep>::lowest(); std::numeric_limits<Rep>::max().
- Note 2: hashable since C++26, as if hashing underlying representation.
- Note 3: some member type aliases:

# Supplementary

- Chrono
- Compile-time rational number
- Time
- Duration
- Time point
- Date and time zone

# Time point

- Essentially, time point is a relative quantity w.r.t. a reference origin.
- That is, we can define it as an epoch (as reference) plus a duration.
- In C++, epoch is defined in clock, which also has a tick resolution (which is still a duration).

Since C++20

# Time point

- We’ll discuss detailed differences of clocks later; let’s go through time point now.
- Ctor:

- You can check the duration since epoch by:

# Time point

- And limited arithmetic operations:
- operator+=/-= a duration;
- operator+/- a duration to get time point; or operator- another time point to get a duration. Duration type is still common type.

- Common type of time_point is also specialized, same as that of duration.
- operator++/--;
- Comparable;
- And: which useToDurationinstead of time point as type template. (e.g. std::chrono::floor<std::chrono::days>(SysTime)

std::chrono::floor<std::chrono::time_point< std::system_clock, std::chrono::days>>(SysTime)).

# Time point

- Explicit cast: must be the same clock.
- As if converting underlying duration by duration_cast.
- And some minor members:
- min/max(): return time point with .time_since_epoch() that is same as duration’s min/max().
- Hashable since C++26, as if hashing duration.
- And some member types:

# Time point

- For example:

time_point is not printable and formattable by default except for specializations; steady_clock doesn’t specialize so you can only print its duration.

- So roughly speaking, time_point is a duration with limited utilities.

# Clock

- Most of clocks define static method now() to get current time point.
- Thus a simple profiling segment would be:

- Then what’s the difference of all clocks?
- Epoch;
- Tick resolution, which is implementation-defined;
- Steady or not;
- How to handle leap second (闰秒), if time is printable.
- Current leap seconds are always positive since earth generally spins slower and slower.
- Note: leap second may be cancelled since 2035 (well, I’m not sure), but the previous leap seconds still exist.

# Clock

- Steady means that external adjustment cannot affect the time.
- For example, auto t1 = now(); auto t2 = now(); may lead to t1 > t2, if user changes system clock of OS.
- Only stdc::steady_clock guarantees steady time.
- stdc::system_clock: clock of operating system.
- Epoch: implementation-defined; regulated to be Unix Time since C++20.
- i.e. 1970/1/1, 00:00:00 UTC (Thursday)
- Leap second: every minute has at most 60 seconds; if a positive leap second happens, then previous second will take longer.
- So every second unevenly shares the time (example later).
- Note: You can get time_t in C (not covered here) from stdc::system_clock.

# Clock

- stdc::steady_clock: monotonic clock with constant tick time.
- Epoch: implementation-defined;
- Steady: yes;
- Not printable, every second evenly shares the time.

- stdc::high_resolution_clock: clock with smallest resolution.
- In current implementations, it’s an alias of either steady_clock or system_clock.

- These three clocks are in C++11; and new clocks are introduced since C++20.

# Clock

- stdc::utc_clock: Coordinated Universal Time (UTC, 协调世界时), a.k.a. Greenwich Mean Time (GMT, 格林尼治平均时间).
- This is the time we use in daily life (after offset by time zone).
- For example, Beijing standard time is just UTC/GMT+8.
- Epoch: Unix Time, i.e. 1970/1/1, 00:00:00 UTC (Thursday).
- Note: the official UTC epoch is 1972/1/1, which isn’t used in utc_clock.
- Leap second: a minute may have 59 seconds or 61 seconds.
- So every second evenly shares the time.
- For example, 2017/1/1 Beijing time has 07:59:60.
- You can convert utc_clock to system_clock to cancel handling of leap second, or vice versa.
- sys_time knows leap second (since its inner duration increases), but it doesn’t tell users. So from_sys can get a utc_time with leap second.

# Clock

- stdc::gps_clock: time used by GPS.
- Epoch: 1980/1/6, 00:00:00 UTC.
- Leap second: every minute has exactly 60 seconds; if a positive leap second happens, then it’s just seen as next second in the next minute.
- So every second evenly shares the time.
- Now (as of 2026), GPS clock is 18 seconds ahead of UTC clock.
- stdc::tai_clock: International Atomic Time.
- Same as GPS time except for epoch (since GPS also uses atomic clock).
- Epoch: 1958/1/1 00:00:00 TAI (1957/12/31 23:59:50 UTC).
- Now (as of 2026), TAI clock is 37 seconds ahead of UTC clock.

- You can convert gps_time/tai_time to utc_time or vice versa.

# Clock

- For example:

(Details covered later)

# Clock

- stdc::file_clock: time used by filesystem (for last modification time).
- It’s completely implementation-defined TrivialClock; C++20 adds some utilities like converter and formatter.

- Before that (i.e. in C++17), it’s not easy to print its time point.
- You have to rely on time_t, and tolerate error between two now().
- Finally, if you just want to convey a “pure time point” without any reference, you can use stdc::local_t as clock.
- Like we do in high-school physics…
- It also acts as a pseudo clock when its reference is not determined, as we’ll show in time zone.

# Clock

- More utilities added in C++20 include:

1. Type aliases:

# Clock

2. I/O for time point: except for stdc::steady_clock / high_resolution_clock / local_t. Example: Japanese

- Formatter: you can use all formats except for %Q/q in duration for (since it still keeps era) time in a day; Besides, it provides more formats for date:2011/9/25 Windows Linux

Windows Linux

Week-based year is like syllabus in University; e.g. you will take exam in Week 17 and 18.

th 2011/9/25 is in 38/39 week in 2011 (depending on how to count a week), th 268day in 2011.

Special note: operator<<(os, sys_time) also has special constraints (require integer representation, unit no longer than 1 day):

# Clock

- Different clocks have different abbr. and can be printed by %Z:
- System clock / UTC clock / file clock: UTC;
- TAI clock: TAI;
- GPS clock: GPS;
- You can also use %z/Oz/Ez to print time zone offset, which will be covered later.
- And the whole time point can also be printed by %c:

- operator<< is just equiv. to {:L%F %T}, i.e. year-month-day hour:minute:second.
- operator<< adds an overload for sys_days so that it only prints {:L%F}.
- For example:

Yes, Greek has Full table on Windows,Difference only liesFull table on Linux, two names for in %O, %E and locale- 2011/9/252011/9/25September. dependent output We use 10:30:59 in Linux.

# Clock

- from*stream accepts similar formats (just sometimes be case-insensitive, and add some %N* for “at most N characters”), so we don’t cover it again.
- And you can still use parse as manipulator.

3. Uniform clock conversion: since C++20 introduces many different clocks, it’s necessary to introduce a converter.

- And we say that you can just use e.g. stdc::from_sys, stdc::to_utc, etc.
- However, it doesn’t directly define arbitrary conversion between two clocks.
- Instead, you can use stdc::clock_cast!
- As we show previously:

- Principle: direct conversion if possible, otherwise use UTC and system clock as hub.

# Clock

- If both types can be converted from & to UTC or system clock:
- Then to_sys and then from_sys, or to_utc and then from_utc.
- If both two ways are legal, then compilation error for ambiguity.
- If source type can be converted to UTC and destination type can be converted from system clock (or vice versa, i.e. to system and from UTC):
- Then to_utc, then UTC clock is converted to system clock (by UTC to_sys), then from_sys;
- Or to_sys, then system clock is converted to UTC clock (by UTC from_sys), then from_utc;
- If both two ways are legal, then compilation error for ambiguity.

- The underlying conversion relies on stdc::clock_time_conversion, which has already been specialized on UTC clock and system clock.
- We don’t cover it in detail here. Check manual if necessary.

# Clock

- And some final notes:

1. Do NOT use non-steady clock for waiting events.

- Reason: Non-monotonicity can make waiting much longer than expected.
- For example: std::condition_variable::wait_for/until();
- Initially, libstdc++ implements it by system_clock, so wait_for(10s) can wait for 10 minutes if external users adjust system clock.
- This is fixed in gcc10 by introducing pthread_cond_clockwait in glibc and using steady_clock.

2. Given a UTC time point, you can check whether it’s a leap second by stdc::get_leap_second_info(utc).

- Which returns

elapsed: sum of all leap seconds since epoch.

# Supplementary

- Chrono
- Compile-time rational number
- Time
- Date and time zone
- Date
- Time zone

# Date

- Though we provide many utilities for time point, we don’t tell a way to make a date.
- Well, you can use duration since epoch to construct a time point, but then you need manual calculation (which is also inaccurate since years and months are average time).

- C++20 provides lots of utilities to help you to do so. th
- For example, if we want to meet on the 5day of every month in 2021:

# Date

- Remember our note?

- So the whole process is:

1. 2021y constructs a class year (instead of a duration like 2021ms).
2. Use operator/ of year and get class year_month.
3. Use operator/ of year_month and get class year_month_day.
4. Use operator+=(const months&) of year_month_day to get date of next month.

# Date

- You can also put day first:
- i.e. day -> month_day -> year_month_day.
- Or month first:
- i.e. month -> month_day -> year_month_day.
- You can also use alias names of months:
- Besides normal day, there also exist three ways to denote a date:
- Last day: year_month_day_last, count backward instead of forward; e.g. 3/31 is the last day of March. th
- Weekday: weekday, i.e. Monday ~ Sunday. The iweekday is denoted by weekday*indexed, and forms (year*)month_weekday.
- Last weekday: weekday*last, count backward instead of forward; e.g. 2026/2/23 is the last Monday of 2026/2. Form (year*)month_weekday_last.

1. year_month / last -> year_month_day_last
2. weekday[index] -> weekday_indexed year_month / weekday_indexed -> year_month_weekday
3. weekday[last] -> weekday_last

# Date

year_month / weekday_last -> year_month_weekday_last

- You can just use them like a normal day:

Index starts from 1 instead of 0

- Such output is not interesting, so you can use special formats: Last day ofFirst MondayLast Monday all months.of all months.of all months.

So different classes have different explanations of operator+=(months)!

Tag type for stdc::last.

# Date

- So now you can understand all the classes with different combinations:

1. Fundamental types share similar methods:

- Ctor: constructed from an unsigned int.
- explicit operator unsigned int (except for weekday).
- year is int for the above two, for existence of BC.
- .ok(): judge whether it exists (details covered later).
- operator==/<=> (with the same class).
- weekday doesn’t have operator<=>.
- operator++/--/+=/-=/+/-.
- Defined on proper duration, e.g. day += days, month += months, year += years.
- formatter/operator<</from_stream.
- Format spec. is same as ones covered in time_point, except that you cannot use absent fields (e.g. illegal to use %Y for day).

# Date

- So why is weekday special?
- Order of weekday is not really uniformly defined…
- Some cultures use Sunday as the last day, while others use it as the first day.
- So you cannot really define a uniform operator<=>. [0, 6], Sunday is 0.

1. C++ provides two getters: [1, 7], Sunday is 7.

- 0 and 7 are also same in ctor.

2. Its arithmetic operations are defined on (mod 7).

- For example, Friday += 2 is Sunday; ++Sunday is Monday; Sunday – Friday == 2; Monday – Friday == 3.
- These integers are actually duration days.

3. Finally, you can construct weekday by time point sys_days or local_days.

- So you can know weekday of that calendar day.

# Date

2. Index weekday by operator[] gives weekday_indexed/last: weekday_indexed weekday_last

- The methods are pretty easy:

- operator<< and default format of weekday are equiv. to %a (i.e. locale-dependent abbr.), and weekday_indexed/last is equiv. to %a[index].
- E.g. Mon[1], Fri[last].

# Date

- Note 1: regulations of .ok():
- Particularly, internal types constrain the range of values. Behavior of out- of-range values is undefined.
- For example, day{1} is valid; day{32} is invalid but well-defined; day{256} is unspecified.
- And consequently, result of day{1} + days{255} is unspecified (instead of definitely invalid).
- Note 2: for an invalid date (i.e. .ok() returns false), operator<< and empty format output “ is not a valid XX” additionally.
- XX can be day/month/year/weekday/index, depending on the type.
- Note 3: arithmetic operations of month are defined on mod 12 .
- But stored months will +1 (i.e. in [1, 12]).

# Date

3. Composite types also share similar methods:

- Getters: get properties from stored fields.
- E.g. for year_month_weekday_last:
- The types are just fundamental types.
- .ok(): judge whether the date can exist.
- E.g. 2021/2/29 (as year_month_day) can’t exist; but 2/29 can (as month_day); 2/31 can’t exist, but 31 can (as day).
- operator==/!= (with the same class).
- (year\_)month_day(\_last) also support operator<=>.
- formatter/operator<<; from*stream only for (year*)month(\_day).
- Similarly, you cannot use absent fields for formatters (e.g. illegal to use %D for year_month).
- operator+/-/+=/-=(years/months), only for year_xxx.
- With different interpretations in different classes, as we mentioned before.

# Date

- Note 1: operator<< and empty format come done to output fundamental types connected by /.
- As we show in the example before:
- So invalid indications will also be connected:

- Note 2: the result date of operator+/-/+=/-= may be invalid, e.g. 2021y/1/31 + months{1} == 2021y/2/31.
- Note 3: the four complete date types can be converted to sys_days or local_days.

# Date

- Adjusted example:

- NOTE AGAIN: first + months is not same as sys_days{ first } + months; durations treat months as average.

# Date

- Another example:

We’ve said that weekday can be constructed from sys_days.

th

- Effects: print Friday that is 13day in a month between 2020 and 2024.

# Date

- Note 4: year_month_day is a special type.

1. It can be constructed from year_month_day_last.
2. It can be constructed from sys_days and local_days.

- By contrast, other types can only be converted to sys_days and local_days.
- So a uniform way to convert other types to year_month_day is by (1) converting to sys_days (2) constructing year_month_day.

3. operator<< and empty format print %F (i.e. %Y-%m-%d) instead of components connected by /.

- When it’s invalid, the printed content doesn’t come done to invalid messages of components either; it just prints “ is not a valid date” additionally.

# Date

- Note 5: year_month_day_last defines .day() additionally, so you can get the last day directly.

- Note 6: year_month defines operator-(year_month, year_month) -> months, so you can get difference in months between them.

- Note again: the specific value is not accurate as duration.

- Note 7: all types are hashable since C++26.

# Supplementary

- Chrono
- Compile-time rational number
- Time
- Date and time zone
- Date
- Time zone

# Time zone

- Different regions have different time zones.
- It’s just conveyed by an offset w.r.t. clock, e.g. UTC+8 for Beijing time.
- So 13:00 Asia/Shanghai is same as 5:00 UTC, or 23:00 Ameria/Chicago (in the last day).
- Some cultures also have summer/winter time (夏令时/冬令时), also called daylight-saving/standard time (日光节约时/标准时).
- But time zones have special characteristics…

1. The offset isn’t necessarily multiple of hours; 15/30/45 minutes are often used too.
2. Abbr. can be ambiguous, e.g. CST can refer to Central Standard Time (for Chicago), China Standard Time or Cuba Standard Time, etc.
3. Time zone can change when countries decide to do so (e.g. when they introduce summer/winter time).

# Daylight saving time

- BTW, a little bit knowledge about daylight saving time (DST).
- Generally, it aims to match time with daylight.
- For example, if daylight appears after 7:00 in winter but after 6:00 in summer, then applying DST will make daylight always appear after 7:00.
- When it goes to summer, time will jump to later hours;
- When it goes to winter, time will jump back to previous hours.

- For example, in some states of America, the second Sunday on March will apply DST, and cancel it in the first Sunday on November.
- E.g. in 2016, 2016-03-13 02:00:00 will jump to 2016-03-13 03:00:00, so 2016-03-13 02:30:00 doesn’t exist locally.
- And 2016-11-06 02:00:00 will jump back to 2016-11-06 01:00:00, so 2016- 11-06 01:30:00 correspond to two UTC time points.
- So a local time can be either absent or ambiguous!

# Time zone

- There are four different kinds of time zone names, which are regulated by IANA (Internet Assigned Numbers Authority):

1. City or country.

- e.g. America/Chicago, Asia/Hong_Kong, Europe/Berlin, Pacific/Honolulu, etc.

2. Offset: starts with Etc/, plus GMT+/-N.

- e.g. Etc/GMT-8 is Beijing time (Yes, Etc/GMT-8 is UTC+8, it needs negation for Unix convention on Etc/GMT).

3. Abbreviations.

- e.g. UTC, GMT, CST, PST.

4. Deprecated entries (no deterministic forms).

- e.g. PST8PDT, US/Hawaii, Canada/Central, Japan.

- The unique names can be used to locate a time zone.

# Time zone

- In C++, stdc::time_zone represents a time zone and stdc::zoned_time represents time attached to a time zone.
- C++ provides a database for you to query a const time_zone\*;

1. current_zone(): get current time zone;
2. locate_zone(name): query time zone from name;

- When no entry is located, std::runtime_error will be thrown.

- And there are two ways to construct a zoned_time:

1. Attach a local time to its zone;
2. Convert a zoned time or system time to a zone.

# Time zone

- For example, if we want to meet every Friday at 18:30 (Beijing time), but there are participants in Los Angeles. Windows
- The time schedule would be:

1. Attach local time with a time zone;2. Convert zoned time to another zone;

Linux

BTW you can also use stdc::floor<stdc::seconds>(…) instead of stdc::time_point_cast.

# Time zone

- Another example: get current time in current time zone.

Convert system time to zoned time

- And you can also convert a zoned_time back to local_time and sys_time:

# Time zone

- Specifically, for ctor of zoned_time:
- Each variant provides two overloads, by name (effectively call locate_zone) or const time_zone\*.

1. Attach local time to a time zone:

What’s this?

- We’ve said that a local time can be ambiguous or absent in DST.
- enum class choose has earliest and latest options, which selects the earliest one or the latest one for an ambiguous time.
- Ambiguous time without choose param, or absent time will throw exception.

# Time zone

2. Convert from sys time: When time zone is not provided, UTC will be used as default.

3. Convert from zoned time:

- Actually it also provides variants with stdc::choose as the last param, but such param has no effect.

# Time zone

- And finally some naïve ctors:
- Equiv. to default constructed time\_ point, i.e. .time_since_epoch() is zero. Again, absent time zone is UTC.

- For assignment:

- The last two will keep the original time zone.
- And since local_time doesn’t have variant with choose, it may throw exception.

# Time zone

- And some other methods for zoned_time:
- .get_time_zone() -> const time_zone\*;
- .get_info() -> stdc::sys_info: get information of time zone for current time.
- It contains:

1. In what range this offset is effective.
2. Offset with system time.
3. Possible DST offset.
4. Abbr. of time zone.

- It can be printed by operator<</format directly.
- For example:Reason: China used DST from 1986 to 1991. Los Angeles, in DST. LA, out of DST. Beijing, no DST. Beijing, with DST. (should be GMT+9, but Windows seems wrong). While Linux is correct:

# Time zone

- Equal comparable: compare both time point and time zone pointer;
- Hashable since C++26;
- operator<< and format: same as outputting the local time, except that:

1. %Z will output abbr. of time zone, which is system-dependent;
2. %z will output offset of time zone in [+/-]HH[MM];

- E.g. Beijing is +0800 (or equivalently 08, +08); in previous sections it’s just +0000 (or 00, +00, -00) for UTC clock, TAI clock, etc.

3. %Oz, %Ez will use [+/-]H[H][:MM], plus other locale-dependent transformation.

- E.g. Beijing is +08:00 (or +8, +08, 8, 08); in previous sections it’s +00:00.

4. The default format is %F %T %Z.

- E.g. 2026-03-21 15:15:06 GMT+8.
- And finally a member type: And a type alias:

# Time zone

- Generally, zoned_time isn’t directly input; but you can input a local time by from_stream and construct zoned_time.

- When %Z is provided, abbrev will be filled; when %z/Oz/Ez is provided, offset will be filled.
- %Z takes the longest sequence of characters in regex [\w|-|+|/] (i.e. A-Z, a-z, 0-9, \_, -, +, /).
- For example:

Note: stdc::parse accepts reference instead of pointer.

# Time zone

- Note 1: %Z and %z are separately processed so it’s possible that the offset is not aligned with the time zone name.
- Note 2: unlike other specifiers, %Z is not essentially inversible.
- That is, usually name output by %Z cannot locate a time zone. Two cases:

1. It’s a pseudo name that’s not a time zone.

- E.g. GPS, TAI.

2. locate_zone() uses name to locate, but %Z outputs abbreviation.

- E.g. our previous example:
- GMT+8 is not a valid time zone name.
- Only occasionally, abbr. is also a unique name (like UTC).
- Solution: output .name() when formatting, or scan the time zone database to let users determine (covered later).

# Time zone

- Some final notes…
- Note 1: Methods of time_zone:
- Most of methods in zoned_time just call them.
- And of course, to_sys has a choose param variant.
- You can see that time_zone also provides .get_info for local_time…
- It returns local_info, which describes the result of converting local_time to sys_time:
- When .result is ambiguous or nonexistent, first is the sys_info for the previous zone and second is for the later one.

# Time zone

- Note 2: to be rigorous, time zone pointers are part of template:

- And all previous claims that return const stdc::time_zone\* essentially return TimeZonePtr.
- Details are covered in homework for not being very important.
- Note 3: currently accessing time zone information in MS-STL is slightly slow, which may be fixed in the future.

# Time zone database

- Finally, we’ll introduce the underlying database.
- C++ standard library regulates to use IANA timezone database, but OS may only adopt it partially.
- It can even be absent in your OS; for example, an embedded system may not care about zone at all so the tailored OS removes it.
- Particularly, Windows before Win10 just has no such database.
- This leads to facts that:

1. When the database doesn’t exist, std::runtime_error will be thrown in locate_zone, etc.
2. For a long-running program, the database can be obsolete and gives wrong time.

- For example, new leap second and new time zone may be absent.

# Time zone database

- Essentially, the program maintains a time zone list.
- You can get it by stdc::get_tzdb_list() -> stdc::tzdb_list.
- Every element in the list is a database stdc::tzdb.
- Previous locate_zone() and current_zone() just use tzdb at the head of the list (equiv. to stdc::get_tzdb()).
- You can update the head by stdc::reload_tzdb(), which will compare the version with stdc::remote_version().
- If the head version is older, the latest one got from remote will be inserted at the front of the list.
- Then for a long-running program, calling reload_tzdb can make locate_zone up-to-date without updating the OS.
- So…why does the standard library maintain a list?

# Time zone database

- Reason: there may exist time_zone\* that refers to entries in the old database in the program.
- Removing it directly can cause illegal access.
- But you can do it manually by tzdb_list methods:

- So these statements are equivalent:

name() is the alternative name, and target() is the original name.

# Time zone database

- Finally members of tzdb:

->sys_seconds

- stdc::leap_second contains information of when leap second happened:

- So we can write a program to check all time zones with some abbr.:

For abbrev = “CST”:

The specific list is system-dependent.

# Supplementary and Summary

Math functions

# Supplementary

- Math functions
- Miscellaneous topics
- Random number

# Math functions

- Just list a table (in <cmath>) with special notes:

Class Functions Notes Trigonometricsin, cos, tan, asin, acos,atan2(y,x)distinguishes𝑦/𝑥and–𝑦/−𝑥. 三角函数atan, atan2 Hyperbolicsinh, cosh, tanh, asinh,N/A 双曲函数acosh, atanh

𝑥 Exponential &exp, exp2, expm1, log,expm1means𝑒−1(exp minus 1), log1p means ln(𝑥 + 1) (ln 1 plus), Logarithmlog2, log1p, log10, logb logb means floor(log2(x)). 指数/对数函数 Powerpow, sqrt, cbrt, hypot cbrtmeans𝑥,hypot(x,y[,z])means 2 22 幂函数hypotenuse (斜边) 𝑥+ 𝑦(+𝑧) Roundingceil, floor, trunc, round,trunc(2.1) == 2,trunc(-2.1) ==-2; round doesn’t consider current rounding mode 舍入nearbyint, rint but nearbyint does; rint == nearbyint except for raising error when integer is not exactly representable in current float.

Not frequently used now since compilers can combine division and modulo automatically to save instructions.

# Math functions

Class Functions Notes Divisionfmod, remainder, remquo,Letquo = x / y,remainderisx- 除法divround(quo)*y,fmodisx-trunc(quo)*y. remquo(x, y, *quo) can be used for periodic functions. div(x, y) -> div_t is for integer to get both remainder and quotient. 𝑒 (De)compositionfrexp, ldexp, scalbn,frexp(x, *e)-> m:𝑥=𝑚∗2,𝑚∈[0.5,1); 𝑒 分解与组合modfldexp(m, e):𝑥=𝑚∗2(scalbnis same, except for n-bit system that 𝑛 ≠ 2); modf(x, \*intptr) -> frac: 𝑥 = int + frac. Classificationfpclassify, isfinite,normal:规格化数。 分类isinf, isnan, isnormalfpclassify: returnintegerfor class. Bit operationssignbit, copysign,signbit(x)-> bool; 比特操作nextafter, nexttowardcopysign(x, y):xcopies sign fromy; nextafter(from, to): return next representable number towards to. Minor difference with nexttoward.

𝐿 2𝑏 1𝑥−𝜇 Given 𝑁(𝜇, 𝜎), Pr 𝐿𝑎≤ 𝑋 ≤ 𝐿𝑏= ׬exp − d𝑥 = 𝐿𝑎2𝜋𝜎 2𝜎 1𝐿𝑏− 𝜇𝐿𝑎− 𝜇1𝐿− 𝜇 erf − erf , Pr 𝑋 ≤ 𝐿 = erf + 1 2 2𝜎 2𝜎 2 2𝜎

# Math functions

Class Functions Notes Comparisonis(greater/less)(equal),N/A islessgreater, isunordered 比较

2𝑥 2 −𝑡 Other-1erf, erfc, tgamma,erf 𝑥 = ׬𝑒d𝑡, which is integral of 𝜋 0 其他-1lgamma Gaussian Distribution 𝑁(0, 0.5) in [−𝑥, 𝑥] (i.e. probability of happening in [−𝑥, 𝑥]).

erfc 𝑥 = 1 − erf(𝑥).

tgamma is gamma function Γ 𝑥 = ∞ 𝑥−1−𝑡 ׬𝑡𝑒d𝑡, lgamma is ln Γ 𝑥 . Other-2fmin, fmax, abs, fdim,fdim(x, y) = max(0, x-y); 其他-2fma, nanfma(x, y, z) = x _ y + z; nan(const char_ arg) converts a string to NaN; the string is used to determine the specific binary (e.g. nan("1") is 0x7ff0000000000001).

# Math functions

- Note 1: these functions often correspond to special instructions, which will be applied by either the standard library or the compiler optimization.
- For example, 1.0f / sqrtf(x) will be optimized to RSQRTSS in Intel;
- Sometimes performance loss may even occur…

1. GNU extension provides sincos, which is historically faster for fsincos instruction in x87 architecture; but it’s slower than separate computation now. 𝑒
2. ldexp(m, e) (i.e. 𝑥 = 𝑚 ∗ 2) can be slower than m \* (1 << e), when SIMD instructions are used.

- Note 2.1: almost all functions have variations with suffix f or l, meaning for float or long double.
- E.g. sinf for float, sinl for long double.
- scalbn is slightly special (though it’s rarely used):

# Math functions

- These functions exist for compatibility with <math.h> in C.
- C doesn’t have function overloading, so it has to provide different names (no suffix means for double).
- C++ actually adds overloads:

For extended floating-point numbers in C++23 <stdfloat>.

- But suffix ones can still be used when you need explicit type control.
- Note 2.2: some functions have variations for with prefix i, l or ll, meaning for returning int, long or long long.
- For example, logb returns floor(log2(x)), ilogb returns int(log2(x)).
- Since it differs on return type, overloading cannot help here.

# Math functions

- A thorough example:
- But most of functions don’t have all these permutations of prefix and suffix…

# Math functions

- Note 2.3: it’s recommended to fully qualify function names (i.e. std::xxx) to prevent ambiguity with C functions. C++
- For example: what does abs(-3.1416) return? only
- Oops, it depends on calling C++ function or C function.
- C doesn’t have abs(float) and introduces fabs!
- So abs(-3.1416) can be 3 or 3.1416.
- Just use std::abs!
- Note 3: there also exist some constants, but std::numeric_limits in <limits> should be used in C++ instead.

# Math functions

- Note 4: error handling of math functions is by either floating- point exception or errno (specific way determined by macro).

floating-point exception can be tested by fetestexcept (details not covered here).

# Math functions

- Note 5: most of these functions become constexpr since C++23.

- Note 6: most of these functions add explicit SIMD overloads since C++26.
- Since C++26 introduces <simd>; but we don’t cover it here.

- Note 7: some special notes on numeric precision…
- Before that, we first introduce unit in the last place (ULP).

# ULP

- ULP means spacing around current floating points.
- i.e. ULP(x) = nextafter(x, +inf) – x for x > 0; ULP(x) = x - nextafter(x, -inf) for x < 0;
- Under IEEE 754, that means:
- Assuming floating-point has p bits for mantissa, and exponent of x is e; 𝑒−𝑝
- For normal numbers, ULP is 2; 𝑒−𝑝+1
- For denormal numbers, ULP is 2.
- Say we want to compute ULP for float x = 1.0f.
- 32-bit float has 23 bits for mantissa; exponent of x is 0 (since 1.0 is 1.0 × 2). −23
- So ULP(1.0f) is 2.
- Using ULP, we can describe computation error for finite precision of floating points.

Catch2 provides WithinULP(x, d) for checking error to be within 𝑑 ⋅ ULP(𝑥).

# ULP

- IEEE-754 guarantees some errors to be within 0.5 ULP:
- Rounding: given any value 𝑥, converting it to floating-point x will choose the closest representable value, i.e. error in 0.5 \* ULP(x).
- Plus: (𝑎 ⊕ 𝑏) − 𝑎 + 𝑏 ∈ 0.5 ULP(𝑎 + 𝑏).
- That is, given computation result of float a + float b (denoted as 𝑎 ⊕ 𝑏), and the real value 𝑎 + 𝑏 (also called infinite-precision value), their space is no more than 0.5 ULP(𝑎 + 𝑏).
- Of course, it assumes 𝑎, 𝑏 are already exactly representable.
- Similarly, minus, multiplication and division are in 0.5 ULP(𝑎 + 𝑏) too.
- Most importantly, IEEE-754 requires sqrt and fma to be in 0.5 ULP too.
- So when you have precision requirement, use fma instead of 𝑎 ⊗ 𝑏 ⊕ 𝑐.

FYI, you can check some extreme error care in graphics.

# Math functions

- Note 8: some functions add computation for complex number in <complex>.
- std::complex<T> is generally like a wrapper for T real, imag; details not covered here.

You can also use user-defined literals.

Methods for std::complex, quite easy.

# Math functions

- Finally, C++17 adds many special math functions.
- Details not covered here; just use it when your task needs.

For example, graphics usually uses Spherical Harmonics, which can be calculated by sph_legendre.

# Numbers

- C++20 adds many math constants in <numbers>.
- namespace is std::numbers.
- When \_v is removed, it means double constant (e.g. std::numbers::e).
- If you want float constant, you need to use e.g. std::numbers::e_v<float>.

# Side note

- Also a side note: C++26 adds some integer arithmetic utilities.
- Saturation: in <numeric>; saturating_xxx<T>(x, y) -> T will saturate computation of x and y, so when the result is out-of-bound, it will be clamped to representable value.
- xxx can be add, sub, mul, div.
- And saturating_cast<T>(x) -> T; by comparison, (T)x will modulo it. E.g. (int8_t)-696 is 72, while saturating_cast<int8_t>(-696) is -128.
- Example:
- Checked: in <stdckdint.h> (which is first accepted in C23); when the computation result of x and y is out-of-bound, ckd_xxx(*result, x, y) -> bool will not set *result, and will return false.
- xxx can be add, sub, mul.

# Supplementary

- Math functions
- Miscellaneous topics
- Random number

# Random number

- C just provides a very simple function rand to get a pseudo- random integer.
- And you can only manipulate it by setting seed using srand.
- But it has many drawbacks…

1. Not thread-safe, so have to add lock to protect;
2. Many math formula use special distributions; providing mere random integers isn’t enough.
3. The implemented algorithm, for backward compatibility, is very naïve and likely to generate low-quality random numbers.

- That is, random number may encounter loop quickly.
- To overcome these problems, C++11 introduces <random>.

Implementations usually just use /dev/urandom or OS random library. But you can use other strings in ctor,

# Random numbere.g. /dev/random, to specify explicitly.

- In physics, there are many “true” random process.
- For example, whether a radioactive particle (放射性粒子) decays is completely random; we can only depict its overall behavior statistically by half-life (半衰期).
- In computers, we may use noise of reverse-biased diodes (反向偏置二极管).
- C++ provides such a “true” random number generator std::random_device.
- Typically, it gets random number from hardware device.
- However, when such true number generator is absent, it can also be implemented as software-based pseudo-random number generator (PRNG).
- You can check it by .entropy() -> double;
- PRNG implementation “will” return 0 while others return positive value.

# Random number

- But typically, performance of std::random_device will degrade sharply when called frequently (when it has to “flush the noise”).
- Thus, it’s usually used to generate a seed for PRNG.
- C++ provides a bunch of PRNGs with different algorithms:

- We’ll briefly introduce these engines later…

# Random number

- These PRNGs generate random integers in uniform distribution;
- But in scientific problems, we usually need random numbers that obey some distribution.
- C++ also provides a lot of distributions that accept PRNGs, and transform the generated number to obey distribution.
- We won’t cover them in detail and check manual when you need.

# Random number

- So a commonly used pattern to generate random number is:

1. Generate a device;
2. Get a seed to initialize PRNG;
3. Create a distribution;
4. Sample the distribution using PRNG.

- Now let’s cover a little bit of details about these utilities…
- But we won’t cover detailed algorithms; if you have special requirements on quality of random number, check them yourself.
- It’s really, really an expert-level work.

# Random number

- For PRNG, they have common interface below:

Ctor: should be copyable; PRNG copies share the same internal state.

op(): generate a random integer in [min, max], and advance to the next state. .discard(unsigned long long n): discard the next n random integers. Though naïvely it can be implemented by looping op() by n times, some engines may have better algorithm.

PRNG can be equality- compared and input/output (I/O format is impl-defined).

# Random number

- Ctor and .seed() accept a seed to (re)set its state.
- There are several variants:

1. Accept a single integer, as we show before.
2. Accept nothing, as if accept a default integer for 1..

- But is it enough?
- NO. 𝑀
- Reason: an integer of 𝑀 bits has only 2states; but some engines have much more state bits (e.g. for std::mt19937, it has 624 32-bit integers).
- Therefore, a variant for accepting a seed range is needed. So:

3. Accept a SeedSequence, which is capable of generating many integers.

- This overload is a template, and the standard library already provides a class std::seed_seq that fulfills SeedSequence.

# Random number

- For example:

- We give it 8 random 32-bit integers for seeding; though it’s still far less than 624, it should be better than only a single integer.

- Therefore, SeedSequence is a mapping that:
- Accepts 𝑀 random bits;
- Output 𝑁 random bits that eliminate bias (as much as possible), which will be used to initialize PRNG status.

# Random number

- std::seed_seq has:

1. Ctor: initialized from a range of 32-bit integers;
2. .generate(begin, end): fill [begin, end) with random integers that eliminate bias.
3. Get status of v.

- Note: it doesn’t guarantee a bijection when 𝑀 = 𝑁 (i.e. the input status space is same as the output space). For instance, given two 32-bit integers, you cannot get every value of 64-bit integers.

# Random number

congruential: 同余

- The basic properties of PRNGs are:

1. std::linear_congruential_engine: a.k.a. LCG, widely used in C rand() implementation; 𝑥𝑖+1= 𝑎 ⋅ 𝑥𝑖+ 𝑐 mod 𝑚, 𝑥0is seed.

- Occupies least space (only one integer 𝑥), moderately fast, worst random 𝑖 quality.

2. std::mersenne_twister_engine: Mersenne Twister algorithm.

- Occupies most space (624 integers for mt19937), very fast, high random quality 𝑀 (cycle is 2− 1 where 𝑀 is Mersenne prime; in mt19937 𝑀 = 19937). Space is n integers of type UIntType.

3. std::subtract_with_carry_engine: subtract with carry algorithm.

- Occupies moderate space, moderately fast, moderate random quality.

Space is r integers of type UIntType, plus a flag (either 0 or 1).

# Random number

- And some PRNG algorithms may need transformation on previous PRNGs, so there are three adaptors:

1. std::discard_block_engine: for every P random numbers, keep only the first R random numbers.
2. std::independent_bits_engine: generate a random number whose last W numeric bits are independent.

- So it can create a larger range of random number, e.g. using a 32-bit engine to generate a 64-bit number.
- The result is stored in UIntType, whose bits should be no less than W.

3. std::shuffle_order_engine: shuffle the random number generated from the engine.

- It keeps a table of size K, fill it will K random numbers from engine; then select from the table randomly as the next random number, and replace that item in the table with a new random number.

It’s an expert-level task to select good parameters for them, so C++ provides lots of predefined aliases:

If you don’t have special requirements, just use std::default_random_engine (usually std::mt19937 in implementations).

# Random number

- Previous PRNGs are all iterative, i.e. 𝑥𝑖+1= 𝑓(𝑥𝑖) to get next random number.
- However, it significantly restricts usage of random number in parallel.
- To overcome that, one way is to use e.g. thread_local PRNGs…
- But, such approach still has many drawbacks:

1. Initializing PRNG is often costly, but every thread has to do so;
2. Weak reproducibility when you add more threads.

- A new thread will introduce a completely different sequence and leads to different numeric results.

3. Multiple good seeds need to be provided.

- Instead, what if we can provide a PRNG with 𝑥𝑖= 𝑓(𝑖)?
- Providing a task id for every thread is a very common pattern in multi- threading tasks, so such PRNG makes the random number easy to get.

# Random number

- C++26 adds a counter-based PRNG std::philox_engine.
- This engine is also widely used in Pytorch on different devices. 𝑛∗sizeof(UIntType)
- Its cycle is 𝑛 ∗ 2.
- philox_engine has state as:

1. An integer counter; to represent a long integer, an array of size std::array<UIntType, n> is used.
2. An integer seed with half bit length, i.e. an array of size n / 2.
3. Buffer b for generated result with same bit length, i.e. an array of size n.
4. An index i to split the buffer and get next random number properly.

- Its operator() -> UIntType returns b[i] and increases i.
- When i reaches n, the philox algorithm runs to generate next b, and the counter is increased by 1.

# Random number

- For example:

1. Ctor: Need 4 / 2 = 2 integers of 32 bits as seed; we provide one and the other will be set as 0.
2. Counter: initialized as all 0.
3. operator(): first generate four 32-bit integers, and consume two here. th

- If we consume 5 integers, then generating the 5integer needs to (1) increase the counter (2) generate the next four 32-bit integers. 4×32
- Now it can provide 4 ⋅ 2random numbers.
- So how does it facilitate parallel tasks?
- C++ provides .set_counter() to offset the counter, so you can divide the numbers into any chunk!

For algorithm details, check this blog or the original paper.

# Random number

- For example, assuming our loop below is parallelized:

Every sub-task occupies the sequence headed by {atom_id, time_step}, so it can freely consume 𝟐×32 4 ⋅ 2random numbers.

- Though other PRNGs can .discard to achieve similar effects, they consume 𝑂(log 𝑛) or 𝑂(𝑛) where 𝑛 is jumping distance; philox_engine does so in 𝑂(1).

# Final Notes

1. About reproducibility:

- PRNG algorithms are strictly regulated, so as long as the seed is the same, PRNG generates the same sequence in all platforms.
- However, distribution algorithms are not regulated, so even if the PRNG is the same, distributions can generate different sequences in different platforms.
- If you want to ensure the same random values everywhere, <random> is not enough.

2. Cryptographical security (密码安全) is not concerned in <random>.

- Cryptographical security means that, even if you know the PRNG algorithm, and you know some part of the sequence, you’re still unable to know the rest of the sequence, or guess PRNG seed in polynomial complexity.
- You should use e.g. OpenSSL for such random sequence.

# Final Notes

3. Some uncovered utilities:

- generate_canonical(): given a PRNG, generate a random floating-point number in [0, 1) with at most Bits (but no more than sizeof(RealType)) randomness.
- Implementations use PRNG to generate random bits.
- For example, program below generates double with 10-bit randomness (i.e. at most 2possible values).

- And some random-related algorithms in <algorithm>:

1. shuffle: shuffle the random-access range with a PRNG.

- URBG means uniform random bits generator, which is equiv. to PRNG in C++ since they generate integers uniformly.

Details of random algorithms are also not strictly regulated, so not reproducible in different platforms.

# Random number

2. sample: since C++17, select n elements from [first, last) randomly to out, each element has equal probability of appearance.

- Every element will appear at most once in the output range.

- The above two also have std::ranges version since C++20.

3. generate_random(range, PRNG[, distribution]): since C++26, std::ranges version only.

- range can also be two params begin, end;
- Effects: fill the range with PRNG or distribution.

Question: why not just std::generate?

The standard encourages standard library implementations to add their .generate_random for PRNGs and distributions if optimizations are possible.

# Random number

- Reason: stdr::generate_random allows for customization.
- For example, some PRNGs or distributions can be vectorized, but the plain fallback loop will suppress it.
- These PRNGs or distributions can provide its member function .generate_random(), and stdr::generate_random will call them.

# Supplementary and Summary

Summary and future prospect

# Course Outline

- Back to our outline in the first lecture…

1. Introduction
2. Basic review & extension
3. Containers
4. Ranges and algorithms

- We’ve talked about how algorithms and containers are implemented; and a bit of C++20 ranges.

5. Lifetime (& Type Safety)

- Lifetime and storage duration, strict aliasing rules, slicing problems, C++- style type conversion, variant and any.

# Course Outline

6. Programming in multiple files

- Basic principles like preprocessor, Translation Unit, ODR, namespace and linkage (and inline), so we can explain why header and source files work.
- A bit of xmake, how to make a library.
- And a bit of C++20 modules.

7. Error Handling

- Starting from C error code, we gradually introduced optional & expected, exceptions, assertions and debug helpers (e.g. stacktrace).
- Note that C++ also wraps error code, and it’s discussed in homework of this lecture (i.e. Lecture 16).
- We emphasized exception safety and talked about copy-and-swap idiom.
- And a bit of Catch2 to do unit test.

# Course Outline

8. String and Stream

- String literals and raw strings, string and string_view, and <charconv>.
- How Unicode works, and how it’s supported in C++.
- Format and print functions, extension to range, how to specialize your own formatter.
- A bit of how stream works.
- And finally regex, but <regex> is not discussed.

9. ~ 11. Move Semantics for 2.5 lectures.

- In Lecture 9, we introduced why we need move semantics, how to write move ctor and assignment, Rule of Five / Zero, moved-from states and some simple algorithms for move semantics.

# Course Outline

- In Lecture 10, we discussed value category (plus decltype), reference qualifier and deducing this, copy elision (RVO, NRVO, implicit move) and some analysis on performance of different types.
- In Lecture 11, we talked about universal reference and perfect forwarding.

11. ~ 12. Templates for 1.5 lectures.

- In Lecture 11, we taught basics of templates, including constexpr (consteval, constinit), specializations and overload resolutions, some tricky details (this->, typename, template, nested specialization) and finally C++20 concept.
- In Lecture 12, we dived into harder parts such as:
- NTTP and template template parameter, type deduction (and CTAD), friend template and lazy instantiation, SFINAE.
- Variadic templates and folder expression.
- Important techniques: CRTP and type erasure.

# Course Outline

13. ~ 14. Multithreading for 1.5 lectures.

- In Lecture 13, we unveiled utilities from low level to high level, from thread (& jthread with stop token handling), synchronizations (semaphores, mutex & locks, condition variable, latch & barrier) to future- promise model, packaged_task and async.
- In Lecture 14, we discussed memory order and atomic variables in detail.

14. Coroutines for 0.5 lectures.

- We gave a brief introduction to coroutines, including how to write them in C++, symmetric transfer and std::generator.

15. Memory management

- We covered object layout, operator new & delete, smart pointers and allocators.

# Course Outline

16. Final

- Finally, we provided some supplementary on filesystem, chrono and math functions, and gave a summary to the whole course.
- Wow, that’s a long journey…
- When I mentioned every part, you can definitely recall lots of knowledge you learnt!
- We’ve covered most of important topics, with some deliberately neglected like ADL.
- The final video length is 46h + 5.5h (this supplementary chapter), which is actually too long (I expect 40h) because I want to cover every detail.
- That’s my fault… If I renew this course, many things will be discarded.
- And as we said, our course only covers until C++23…

# Future C++

- Though C++ provides powerful utilities, it’s still evolving and lots of proposals are submitted every month.
- Particularly, C++26 will bring you these important features:
- We’ve mentioned many, like \_ for placeholder name, rcu & hazard pointer, variadic friends, copyable_function & function_ref, constexpr placement new, indirect & polymorphic, inplace_vector (mentioned in homework), etc. Besides:
- Pack indexing:

# Reflection

- Example: universal formatter to print members. template foris called expansion statement, which is also introduced in C++26..

# Contract

- The finally decided semantics include:

# Execution

- A general and uniform framework for concurrency.
- In <execution>, namespace std::execution.

# And many others…

- Standard library hardening: given special compiler arguments (e.g. -fharden), the standard library implementation should always check conditions!
- For example, vector::operator[]:

- It’s UB when index is out-of-bound; but hardening requires the implementation to check the precondition.
- Really nice feature for safety!
- And note: language UB and library UB are different; plenty of language UBs are not detectable but most of library UBs are detectable.

# And many others…

- Task type for coroutine:

- We’ve implemented a tiny Task in Coroutine, and the standard library just evolves it further and migrates it into std::execution.

1. stdexec::just(0) is a sender that can be co_awaited.
2. Its .await_resume returns its parameter 0.
3. co_return 0 is processed by stdexec::task::promise_type::return_value.

- Parallel Range algorithm;
- We’ve said that parallel algorithms don’t have std::ranges version;
- Since C++26, they are added for random-access range.

# And many others…

- SIMD
- submdspan
- User-generated static_assert messages.
- Erroneous behavior for uninitialized variables.
- Erroneous behavior is new behavior that is well-defined (so compilers cannot do wrong assumptions and make aggressive optimizations like UB), but is essentially incorrect.
- This restricts surprise in “undefined” results.
- If you really want it uninitialized, use attributes:

现代C++基础 Modern C++ Basics

# That’s ALL!

Jiaming Liang, undergraduate from Peking University

Postgraduate from PKU since 2024.9 :-)

# Next lecture?

- Well, our lectures on “Modern C++ Basics” have ended…
- I really appreciate audiences who persist in learning my course and reach here!
- Though this series “goes beyond its lifetime”, all knowledge has been passed to your data member and will continue their adventure with you.

- You’ll definitely learn more and more in C++…
- Just believe in yourself and forge ahead!
- And it’s my honor to become part of this lifelong journey.

- This channel will also continue to produce more contents on interesting and brand-new topics about C++, when I have enough spare time.
- Really thank you, and see you in the next video ☺.
