#ifndef _A_ASSETMANAGER_HPP_
#define _A_ASSETMANAGER_HPP_
#include <vector>
#include "Assets.hpp"

class AssetManager {
	private:
		std::vector<Extension> _extensions;
		std::vector<Trigger> _triggers;
		std::vector<Constant> _constants;
		std::vector<Sound> _sounds;
		std::vector<Sprite> _sprites;
		std::vector<Background> _backgrounds;
		std::vector<Path> _paths;
		std::vector<Script> _scripts;
		std::vector<Font> _fonts;
		std::vector<Timeline> _timelines;
		std::vector<Object> _objects;
		std::vector<Room> _rooms;
		std::vector<IncludeFile> _includeFiles;

	public:
		AssetManager();
		~AssetManager();

		void Clear();

		void ReserveExtensions(unsigned int count);
		void ReserveTriggers(unsigned int count);
		void ReserveConstants(unsigned int count);
		void ReserveSounds(unsigned int count);
		void ReserveSprites(unsigned int count);
		void ReserveBackgrounds(unsigned int count);
		void ReservePaths(unsigned int count);
		void ReserveScripts(unsigned int count);
		void ReserveFonts(unsigned int count);
		void ReserveTimelines(unsigned int count);
		void ReserveObjects(unsigned int count);
		void ReserveRooms(unsigned int count);
		void ReserveIncludeFiles(unsigned int count);

		Extension* AddExtension();
		Trigger* AddTrigger();
		Constant* AddConstant();
		Sound* AddSound();
		Sprite* AddSprite();
		Background* AddBackground();
		Path* AddPath();
		Script* AddScript();
		Font* AddFont();
		Timeline* AddTimeline();
		Object* AddObject();
		Room* AddRoom();
		IncludeFile* AddIncludeFile();

		Extension* GetExtension(unsigned int index);
		Trigger* GetTrigger(unsigned int index);
		Constant* GetConstant(unsigned int index);
		Sound* GetSound(unsigned int index);
		Sprite* GetSprite(unsigned int index);
		Background* GetBackground(unsigned int index);
		Path* GetPath(unsigned int index);
		Script* GetScript(unsigned int index);
		Font* GetFont(unsigned int index);
		Timeline* GetTimeline(unsigned int index);
		Object* GetObject(unsigned int index);
		Room* GetRoom(unsigned int index);
		IncludeFile* GetIncludeFile(unsigned int index);

		inline unsigned int GetExtensionCount() { return (unsigned int)_extensions.size(); }
		inline unsigned int GetTriggerCount() { return (unsigned int)_triggers.size(); }
		inline unsigned int GetConstantCount() { return (unsigned int)_constants.size(); }
		inline unsigned int GetSoundCount() { return (unsigned int)_sounds.size(); }
		inline unsigned int GetSpriteCount() { return (unsigned int)_sprites.size(); }
		inline unsigned int GetBackgroundCount() { return (unsigned int)_backgrounds.size(); }
		inline unsigned int GetPathCount() { return (unsigned int)_paths.size(); }
		inline unsigned int GetScriptCount() { return (unsigned int)_scripts.size(); }
		inline unsigned int GetFontCount() { return (unsigned int)_fonts.size(); }
		inline unsigned int GetTimelineCount() { return (unsigned int)_timelines.size(); }
		inline unsigned int GetObjectCount() { return (unsigned int)_objects.size(); }
		inline unsigned int GetRoomCount() { return (unsigned int)_rooms.size(); }
		inline unsigned int GetIncludeFileCount() { return (unsigned int)_includeFiles.size(); }
};

#endif