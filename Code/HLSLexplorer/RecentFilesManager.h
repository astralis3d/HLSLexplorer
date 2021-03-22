#pragma once

#include <vector>
#include <string>

class CRecentFilesManager
{
	enum { MAX_RECENT_FILES = 8 };
	typedef std::vector<std::string> TVecString;

public:
	CRecentFilesManager();
	~CRecentFilesManager();

	const TVecString& RecentFiles() const;
	unsigned int Count() const;
	
	bool Empty() const;

	void AddRecent(const std::string& recentFile);

	void EraseByIndex(unsigned int index);
	void ClearAll();
	
	bool LoadFromFile(const std::string& file);
	bool SaveToFile(const std::string& file) const;

private:
	TVecString m_recentFileList;
	unsigned int m_nNumRecentFiles;
};