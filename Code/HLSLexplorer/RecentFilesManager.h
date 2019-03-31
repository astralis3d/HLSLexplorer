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

	unsigned int Count() const { return m_nNumRecentFiles; }
	
	bool Empty() const;
	bool Contains(const std::string& name) const;

	void AddRecent(const std::string& recentFile);

	void EraseByIndex(unsigned int index);
	void ClearAll();
	
	bool LoadFromFile(const std::string& file);
	bool SaveToFile(const std::string& file);

	TVecString m_recentFileList;

private:
	unsigned int m_nNumRecentFiles;
};