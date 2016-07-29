SteamItemDropIdler v2.03

All this info available on: https://github.com/Ne3tCode/SteamItemDropIdler/wiki

* Features
	- Works without Steam
	- Steam Guard support
	- Steam Family View support
	- Steam Mobile Authenticator support + 2FA token generation
	- Multiple instance support
	- Auto add free game license (for games that you idle only)
	- Auto reconnect if connection to Steam servers is lost
	- Command line parameter support
	- Can be used for mass idling (low RAM usage)

* Supported games
	- Team Fortress 2
	- Games that use the Inventory Service Steam API

* How to use
	1. Download and extract the release to some folder from here if you have not already: https://github.com/Ne3tCode/SteamItemDropIdler/releases
	2. Make sure Steam is not running. SteamItemDropIdler may not work properly without this step
	3. If you have Steam installed, rename your Steam folder to something else. For example, if your Steam installation is in "C:\Program Files (x86)\Steam", rename it to "C:\Program Files (x86)\Steam123". SteamItemDropIdler may not work properly without this step
	4. Run SteamItemDropIdler
	5. Enter your Steam information, the AppID of the game and the Drop List Definition that you want to idle for. You can find those here: https://github.com/Ne3tCode/SteamItemDropIdler/wiki/Drop-List-Definitions
	6. It should say "Item drop idling is now in progress"
	7. Keep SteamItemDropIdler open and wait till you get items! To stop, press Ctrl+C and then Enter
	
	Note: Do not worry if you get "BeginAuthSession callback failed (5)", everything is working correctly.
