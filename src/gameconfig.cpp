#include "gameconfig.h"
#include "addresses.h"

CGameConfig::CGameConfig(const std::string& gameDir, const std::string& path)
{
	this->_gameDir = gameDir;
	this->_path = path;
	this->_kv = new KeyValues("Games");
}

bool CGameConfig::Init(IFileSystem *filesystem, char *conf_error, int conf_error_size)
{
	if (!_kv->LoadFromFile(filesystem, _path.c_str(), nullptr))
	{
		snprintf(conf_error, conf_error_size, "Failed to load gamedata file");
		return false;
	}

	const KeyValues* game = _kv->FindKey(_gameDir.c_str());
	if (game)
	{
#if defined _LINUX
		const char* platform = "linux";
#else
		const char* platform = "windows";
#endif

		const KeyValues* offsets = game->FindKey("Offsets");
		if (offsets)
		{
			FOR_EACH_SUBKEY(offsets, it)
			{
				_offsets[it->GetName()] = it->GetInt(platform, -1);
			}
		}

		const KeyValues* signatures = game->FindKey("Signatures");
		if (signatures)
		{
			FOR_EACH_SUBKEY(signatures, it)
			{
				_libraries[it->GetName()] = std::string(it->GetString("library"));
				_signatures[it->GetName()] = std::string(it->GetString(platform));
			}
		}

		const KeyValues* patches = game->FindKey("Patches");
		if (patches)
		{
			FOR_EACH_SUBKEY(patches, it)
			{
				_patches[it->GetName()] = std::string(it->GetString(platform));
			}
		}
	}
	else
	{
		snprintf(conf_error, conf_error_size, "Failed to find game: %s", _gameDir.c_str());
		return false;
	}
	return true;
}

CGameConfig::~CGameConfig()
{
	delete _kv;
}

const std::string CGameConfig::GetPath()
{
	return _path;
}

const char *CGameConfig::GetSignature(const std::string& name)
{
	auto it = _signatures.find(name);
	if (it == _signatures.end())
	{
		return nullptr;
	}
	return it->second.c_str();
}

const char *CGameConfig::GetPatch(const std::string& name)
{
	auto it = _patches.find(name);
	if (it == _patches.end())
	{
		return nullptr;
	}
	return it->second.c_str();
}

int CGameConfig::GetOffset(const std::string& name)
{
	auto it = _offsets.find(name);
	if (it == _offsets.end())
	{
		return -1;
	}
	return it->second;
}

const char *CGameConfig::GetLibrary(const std::string& name)
{
	auto it = _libraries.find(name);
	if (it == _libraries.end())
	{
		return nullptr;
	}
	return it->second.c_str();
}

CModule **CGameConfig::GetModule(const char *name)
{
	const char *library = this->GetLibrary(name);
	if (library == NULL)
		return NULL;

	if (strcmp(library, "engine") == 0)
		return &modules::engine;
	else if (strcmp(library, "server") == 0)
		return &modules::server;
	else if (strcmp(library, "client") == 0)
		return &modules::client;
	else if (strcmp(library, "vscript") == 0)
		return &modules::vscript;
	else if (strcmp(library, "tier0") == 0)
		return &modules::tier0;
#ifdef _WIN32
	else if (strcmp(library, "hammer") == 0)
		return &modules::hammer;
#endif
	return NULL;
}

bool CGameConfig::IsSymbol(const char *name)
{
	const char *sigOrSymbol = this->GetSignature(name);
	if (sigOrSymbol == NULL || strlen(sigOrSymbol) <= 0)
	{
		Panic("Missing signature or symbol\n", name);
		return false;
	}
	return sigOrSymbol[0] == '@';
}

const char* CGameConfig::GetSymbol(const char *name)
{
	const char *symbol = this->GetSignature(name);

	if (symbol == NULL || strlen(symbol) <= 1)
	{
		Panic("Missing symbol\n", name);
		return NULL;
	}
	return symbol + 1;
}

// Static functions
std::string CGameConfig::GetDirectoryName(const std::string &directoryPathInput)
{
    std::string directoryPath = std::string(directoryPathInput);

    size_t found = std::string(directoryPath).find_last_of("/\\");
    if (found != std::string::npos)
    {
        return std::string(directoryPath, found + 1);
    }
    return "";
}

int CGameConfig::HexStringToUint8Array(const char* hexString, uint8_t* byteArray, size_t maxBytes)
{
    if (hexString == NULL) {
        printf("Invalid hex string.\n");
		return -1;
	}

    size_t hexStringLength = strlen(hexString);
    size_t byteCount = hexStringLength / 4; // Each "\\x" represents one byte.

    if (hexStringLength % 4 != 0 || byteCount == 0 || byteCount > maxBytes) {
        printf("Invalid hex string format or byte count.\n");
        return -1; // Return an error code.
    }

    for (size_t i = 0; i < hexStringLength; i += 4) {
        if (sscanf(hexString + i, "\\x%2hhX", &byteArray[i / 4]) != 1) {
            printf("Failed to parse hex string at position %zu.\n", i);
            return -1; // Return an error code.
        }
    }

    byteArray[byteCount] = '\0'; // Add a null-terminating character.

    return byteCount; // Return the number of bytes successfully converted.
}

byte *CGameConfig::HexToByte(const char *src)
{
	if (src == NULL || strlen(src) <= 0)
	{
		Panic("Invalid hex string\n");
		return NULL;
	}

	size_t maxBytes = strlen(src) / 4;
	uint8_t *dest = new uint8_t[maxBytes];
	int byteCount = CGameConfig::HexStringToUint8Array(src, dest, maxBytes);
	if (byteCount <= 0)
	{
		Panic("Invalid hex format %s\n", src);
		return NULL;
	}
	return (byte *)dest;
}

void *CGameConfig::ResolveSignature(const char *name)
{
	CModule **module = this->GetModule(name);
	if (!module || !(*module))
	{
		Panic("Invalid Module %s\n", name);
		return NULL;
	}

	void *address = nullptr;
	if (this->IsSymbol(name))
	{
		const char *symbol = this->GetSymbol(name);
		if (symbol == NULL)
		{
			Panic("Invalid symbol for %s\n", name);
			return NULL;
		}
		address = dlsym((*module)->m_hModule, symbol);
	}
	else
	{
		const char *signature = this->GetSignature(name);
		if (signature == NULL)
		{
			Panic("Failed to find signature for %s\n", name);
			return NULL;
		}

		byte *pSignature = this->HexToByte(signature);
		if (pSignature == NULL)
			return NULL;
		address = (*module)->FindSignature(pSignature);
	}

	if (address == NULL)
	{
		Panic("Failed to find address for %s\n", name);
		return NULL;
	}
	return address;
}
