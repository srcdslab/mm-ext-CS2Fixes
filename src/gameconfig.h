#pragma once

#include "KeyValues.h"

#include <string>
#include <unordered_map>

class CModule;

class CGameConfig
{
public:
	CGameConfig(const std::string& gameDir, const std::string& path);
	~CGameConfig();

	bool Init(IFileSystem *filesystem, char *conf_error, int conf_error_size);
	const std::string GetPath();
	const char *GetLibrary(const std::string& name);
	const char *GetSignature(const std::string& name);
	const char* GetSymbol(const char *name);
	const char *GetPatch(const std::string& name);
	int GetOffset(const std::string& name);
	void* GetAddress(const std::string& name, void *engine, void *server, char *error, int maxlen);
	CModule **GetModule(const char *name);
	bool IsSymbol(const char *name);
	static std::string GetDirectoryName(const std::string &directoryPathInput);
	static int HexStringToUint8Array(const char* hexString, uint8_t* byteArray, size_t maxBytes);
	static byte *HexToByte(const char *src);
	void *ResolveSignature(const char *name);

private:
	std::string _gameDir;
	std::string _path;
	KeyValues* _kv;
	std::unordered_map<std::string, int> _offsets;
	std::unordered_map<std::string, std::string> _signatures;
	std::unordered_map<std::string, void*> _addresses;
	std::unordered_map<std::string, std::string> _libraries;
	std::unordered_map<std::string, std::string> _patches;	
};
