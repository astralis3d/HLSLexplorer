#include "PCH.h"
#include "RecentFilesManager.h"
#include <fstream>

CRecentFilesManager::CRecentFilesManager()
	: m_nNumRecentFiles(0)
{

}

//-----------------------------------------------------------------------------
CRecentFilesManager::~CRecentFilesManager()
{
	m_recentFileList.clear();
	m_nNumRecentFiles = 0;
}

//-----------------------------------------------------------------------------
unsigned int CRecentFilesManager::Count() const
{
	return m_nNumRecentFiles;
}

//-----------------------------------------------------------------------------
bool CRecentFilesManager::Empty() const
{
	return m_recentFileList.empty();
}

//-----------------------------------------------------------------------------
bool CRecentFilesManager::Contains( const std::string& name ) const
{
	for (const auto& x : m_recentFileList )
	{
		if (name == x)
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
void CRecentFilesManager::AddRecent( const std::string& recentFile )
{
	if ( Contains(recentFile) )
	{
		// if it's already within list, remove it from current place...
		auto it = std::find( std::begin(m_recentFileList), std::end(m_recentFileList), recentFile );
		m_recentFileList.erase(it);

		// ...and move it to the top
		m_recentFileList.insert( std::begin(m_recentFileList), recentFile );
	}
	else
	{
		// new item, insert to the top
		m_recentFileList.insert( std::begin(m_recentFileList), recentFile );
		m_nNumRecentFiles++;
	}

	// If too many elements, remove the last one.
	if (m_nNumRecentFiles > MAX_RECENT_FILES)
	{
		m_recentFileList.erase( std::end(m_recentFileList) - 1 );
		m_nNumRecentFiles = MAX_RECENT_FILES;
	}
}

//-----------------------------------------------------------------------------
void CRecentFilesManager::EraseByIndex( unsigned int index )
{
	const unsigned int maxIndex = m_nNumRecentFiles - 1;
	if (index > maxIndex)
	{
		return;
	}

	m_recentFileList.erase( std::begin(m_recentFileList) + index );
	m_nNumRecentFiles--;
}

//-----------------------------------------------------------------------------
void CRecentFilesManager::ClearAll()
{
	m_recentFileList.clear();
	m_nNumRecentFiles = 0;
}

//-----------------------------------------------------------------------------
bool CRecentFilesManager::LoadFromFile( const std::string& file )
{
	std::ifstream fin( file );
	if (!fin.is_open())
	{
		return false;
	}

	std::string recentFilePath;

	while ( std::getline(fin, recentFilePath) )
	{
		m_recentFileList.push_back(recentFilePath);
		m_nNumRecentFiles++;
	}

	fin.close();

	return true;
}

//-----------------------------------------------------------------------------
bool CRecentFilesManager::SaveToFile( const std::string& file ) const
{
	std::ofstream fout;
	fout.open(file, std::ofstream::out | std::ofstream::trunc);
	if (!fout)
	{
		return false;
	}

	
	for (const auto& x : m_recentFileList )
	{
		fout << x << std::endl;
	}

	fout.close();

	return false;
}

//-----------------------------------------------------------------------------
const CRecentFilesManager::TVecString& CRecentFilesManager::RecentFiles() const
{
	return m_recentFileList;
}
