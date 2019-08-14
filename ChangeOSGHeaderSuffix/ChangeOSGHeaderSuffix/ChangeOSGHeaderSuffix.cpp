// ChangeOSGHeaderSuffix.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

// ChangeOSGHeader.cpp : 定义控制台应用程序的入口点。
//---用来自动修改osg源码库的头文件不加.h后缀的问题


#include <iostream>
#include <fstream>
#include <string>
#include <deque>
#include <algorithm>
#include<tchar.h>


#include "StaticUtils.h"
using namespace StaticUtils;

const std::string gAppendHeader = ".h";
const std::string gCpp = ".cpp";
const std::string gTxt = ".txt";
const std::string gCmakeList = "CMakeLists.txt";
const std::string CPP = ".c++";
const std::string gOpenThreads = "OpenThreads";
const std::string gOSG = "osg";
const std::string gLeft_ = "<";
const std::string gRight = ">";
const std::string gInclude = "#include";
const std::string newline = "\n";
const std::string strReadMe = "README";
const std::string strCMakeHeader = "SET(TARGET_H";
const std::string strEnd = ")";
auto folderSuccessFileNum = 0;//每个文件夹完成的文件

void ModifierCmakeLists(const std::string &filepath, std::string &outStr)
{
	std::ifstream in;
	char line[1024] = { '\0' };
	in.open(filepath.c_str());
	bool start = false;
	while ((in.getline(line, sizeof(line)).good()))
	{
		std::string strLine = StringHelper::TrimCharToString(line);
		if (strLine == strCMakeHeader)
		{
			start = true;
			outStr += line + newline;
		}
		else if (strLine == strEnd)
		{
			outStr += line + newline;
			start = false;
		}
		else if (start)
		{	//包含.h后缀
			//小括号与.h在同一行
			if (strLine.length() == 0)
				outStr += newline;
			else
			{
				if (strLine.find('.') != -1)
				{
					if (strLine.find(')') != -1)
						start = false;
					outStr += line + newline;
				}
				else
					outStr += line + gAppendHeader + newline;
			}
		}
		else
		{
			outStr += line + newline;
		}
	}
}

//将.h与.cpp中的osg相关引用头文件加上.h后缀
void ReadAndModifier(const std::string& filepath, std::string& outStr)
{
	std::ifstream in;
	char line[1024] = { '\0' };
	in.open(filepath.c_str());
	while ((in.getline(line, sizeof(line)).good()))
	{
		std::string strLine = StringHelper::TrimCharToString(line);
		std::string::size_type whiteIndex = strLine.find_first_of(str_whitespace);
		std::string::size_type leftIndex = strLine.find_first_of(gLeft_);
		if (leftIndex != std::string::npos&&leftIndex >= 8)
		{
			std::string include = strLine.substr(0, 8);
			if (include == gInclude)
			{
				std::string strOSG = strLine.substr(strLine.find_first_of(gLeft_) + 1, 3);
				std::string strOPENTHREADS = strLine.substr(strLine.find_first_of(gLeft_) + 1, 11);
				if (strOSG == gOSG || strOPENTHREADS == gOpenThreads)
				{
					std::string::size_type rightPos = strLine.find_last_of(gRight);
					std::string pre = strLine.substr(0, rightPos);
					if (strLine.substr(rightPos - 2, 2) != gAppendHeader)
					{
						std::string strModifier = pre + gAppendHeader + gRight;
						outStr += strModifier + newline;
					}
					else
					{
						outStr += line + newline;
					}
				}
				else
				{
					outStr += line + newline;
				}
			}
			else
			{
				outStr += line + newline;
			}
		}
		else
		{
			outStr += line + newline;
		}
	}
}

void GetAndModifier(const std::string& strFolder)
{
	std::string dir = strFolder + pFolders;
	intptr_t handle;
	_finddata_t findData;
	handle = _findfirst(dir.c_str(), &findData);
	if (handle == -1)
	{
		std::cout << "访问文件夹  " << strFolder << "  失败！\n";
		return;
	}
	std::string mMergeStr;//输出字符串
	std::string mOutFileName;//输出文件名
	do
	{
		std::string fileName = findData.name;
		if (fileName != "." && fileName != ".." && !(findData.attrib&_A_SUBDIR))
		{
			mMergeStr.clear();
			std::string::size_type hFile = fileName.find(".");//需要显示文件后缀
			if (hFile == std::string::npos)
			{
				std::string hFilePath = strFolder + pReplace__ + fileName + gAppendHeader;
				mOutFileName = FileOp::CreateAscFile(hFilePath);
				std::string sourceFile = strFolder + pReplace__ + fileName;
				ReadAndModifier(sourceFile, mMergeStr);
				FileOp::WriteAscFile(hFilePath, mMergeStr);
				++folderSuccessFileNum;
			}
			else
			{
				std::string suffix = fileName.substr(hFile);
				if (suffix == gCpp || suffix == gAppendHeader || suffix == CPP)
				{
					std::string sourceFile = strFolder + pReplace__ + fileName;
					ReadAndModifier(sourceFile, mMergeStr);
					FileOp::WriteAscFile(sourceFile, mMergeStr);
					++folderSuccessFileNum;
				}
				else if (suffix == gTxt && fileName == gCmakeList)
				{
					std::string sourceFile = strFolder + pReplace__ + fileName;
					ModifierCmakeLists(sourceFile, mMergeStr);
					FileOp::WriteAscFile(sourceFile, mMergeStr);
				}
			}
		}
	} while (_findnext(handle, &findData) == 0);
}

void ChangeHeader(const std::string path)
{
	std::deque<std::string> folderDeque;
	FileOp::GetSubFolder(path, folderDeque);
	std::for_each(folderDeque.begin(), folderDeque.end(), [](std::string& pPath) {
		folderSuccessFileNum = 0;
		GetAndModifier(pPath);
		std::cout << "文件夹：" << pPath << "成功 " << folderSuccessFileNum << std::endl;
	});
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 2)
	{
		std::cout << "not enough parameters!\n";
		return 0;
	}


	std::string inPath = argv[1];
	ChangeHeader(inPath);

	std::cout << "GAME OVER!\n";
#ifdef DEBUG
	system("pause");
#endif

	return 0;
}


