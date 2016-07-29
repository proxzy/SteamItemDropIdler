#include "stdafx.h"

#define SIDI_NAME "Steam Item Drop Idler"
#define SIDI_VERSION "2.03"

const uint32 k_unGCProtoBufFlag = 0x80000000;

const int k_EMsgGCClientWelcome = 4004;
const int k_EMsgGCClientHello = 4006;

CSteamAPILoader g_steamAPILoader;
bool g_bKeepRunning = true;

void ctrl_c_handler( int sig )
{
	signal(sig, SIG_IGN);
	printf( "You pressed Ctrl+C\n" );
	g_bKeepRunning = false;
}

void shutdown( int code )
{
	printf( "Press enter to exit...\n" );
	getchar();
	exit(code);
}

int main( int argc, char* argv[] )
{
	HANDLE hConsole = GetStdHandle( STD_OUTPUT_HANDLE );
	char consoleTitle[256];
	sprintf_s( consoleTitle, sizeof( consoleTitle ), "%s (v%s)", SIDI_NAME, SIDI_VERSION );
	SetConsoleTitleA( consoleTitle );

	SetConsoleTextAttribute( hConsole, FOREGROUND_GREEN );
	printf( "--- Originally made by kokole. Modified by Nephrite ---\n" );
	SetConsoleTextAttribute( hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE );

	char steamAccountName[65];
	char steamAccountPassword[65];
	AppId_t appID;
	SteamItemDef_t dropListDefinition;

	if ( argc == 5 )
	{
		if ( strcpy_s( steamAccountName, sizeof( steamAccountName ), argv[1] ) != 0 )
		{
			printf( "Account name is too long" );
			shutdown( 1 );
		}

		if ( strcpy_s( steamAccountPassword, sizeof( steamAccountPassword ), argv[2] ) != 0 )
		{
			printf( "Password is too long" );
			shutdown( 1 );
		}

		sscanf( argv[3], "%u", &appID );
		sscanf( argv[4], "%d", &dropListDefinition );

		memset( argv[1], 0, strlen( argv[1] ) );
		memset( argv[2], 0, strlen( argv[2] ) );

		printf( "Steam account name: %s\n", steamAccountName );
		printf( "Steam account password: **HIDDEN**\n" );
		printf( "AppID: %u\n", appID );
		printf( "Drop list definition: %d\n", dropListDefinition );
	}
	else
	{
		printf( "Enter your Steam account name: " );
		fgets( steamAccountName, sizeof( steamAccountName ), stdin );
		if ( steamAccountName[strlen( steamAccountName ) - 1] == '\n' ) {
			steamAccountName[strlen( steamAccountName ) - 1] = '\0';
		}
		fflush( stdin );

		printf( "Enter your Steam account password: " );
		fgets( steamAccountPassword, sizeof( steamAccountPassword ), stdin );
		if ( steamAccountPassword[strlen( steamAccountPassword ) - 1] == '\n' ) {
			steamAccountPassword[strlen( steamAccountPassword ) - 1] = '\0';
		}
		fflush( stdin );

		printf( "Enter the AppID: " );
		if ( scanf( "%u", &appID ) < 1 )
		{
			printf( "Invalid AppID\n" );
			shutdown( 1 );
		}
		fflush( stdin );

		printf( "Enter the drop list definition: " );
		if ( scanf( "%d", &dropListDefinition ) < 1 )
		{
			printf( "Invalid Definition ID\n" );
			shutdown( 1 );
		}
		fflush( stdin );
	}

	if ( strlen( steamAccountName ) == 0 || strlen( steamAccountPassword ) == 0 )
	{
		printf( "Account name and password cannot be empty\n" );
		shutdown( 1 );
	}

	sprintf_s( consoleTitle, sizeof( consoleTitle ), "%s (%s)", SIDI_NAME, steamAccountName );
	SetConsoleTitleA( consoleTitle );

	// load steam stuff
	CreateInterfaceFn steam3Factory = g_steamAPILoader.GetSteam3Factory();
	if ( !steam3Factory )
	{
		printf( "GetSteam3Factory failed\n" );
		shutdown( 1 );
	}

	IClientEngine* clientEngine = (IClientEngine*)steam3Factory( CLIENTENGINE_INTERFACE_VERSION, NULL );
	if ( !clientEngine )
	{
		printf( "clientEngine is null\n" );
		shutdown( 1 );
	}

	ISteamClient017* steamClient = (ISteamClient017*)steam3Factory( STEAMCLIENT_INTERFACE_VERSION_017, NULL );
	if ( !steamClient )
	{
		printf( "steamClient is null\n" );
		shutdown( 1 );
	}

	HSteamPipe hSteamPipe;
	HSteamUser hSteamUser = clientEngine->CreateLocalUser( &hSteamPipe, k_EAccountTypeIndividual );
	if ( !hSteamPipe || !hSteamUser )
	{
		printf( "CreateLocalUser failed (1)\n" );
		shutdown( 1 );
	}

	IClientBilling* clientBilling = clientEngine->GetIClientBilling( hSteamUser, hSteamPipe, CLIENTBILLING_INTERFACE_VERSION );
	if ( !clientBilling )
	{
		printf( "clientBilling is null\n" );
		shutdown( 1 );
	}

	IClientFriends* clientFriends = clientEngine->GetIClientFriends( hSteamUser, hSteamPipe, CLIENTFRIENDS_INTERFACE_VERSION );
	if ( !clientFriends )
	{
		printf( "clientFriends is null\n" );
		shutdown( 1 );
	}

	IClientUser* clientUser = clientEngine->GetIClientUser( hSteamUser, hSteamPipe, CLIENTUSER_INTERFACE_VERSION );
	if ( !clientUser )
	{
		printf( "clientUser is null\n" );
		shutdown( 1 );
	}

	IClientUtils* clientUtils = clientEngine->GetIClientUtils( hSteamPipe, CLIENTUTILS_INTERFACE_VERSION );
	if ( !clientUtils )
	{
		printf( "clientUtils is null\n" );
		shutdown( 1 );
	}

	ISteamGameCoordinator001* steamGameCoordinator = (ISteamGameCoordinator001*)steamClient->GetISteamGenericInterface( hSteamUser, hSteamPipe, STEAMGAMECOORDINATOR_INTERFACE_VERSION_001 );
	if ( !steamGameCoordinator )
	{
		printf( "steamGameCoordinator is null\n" );
		shutdown( 1 );
	}

	ISteamInventory001* steamInventory = (ISteamInventory001*)steamClient->GetISteamInventory( hSteamUser, hSteamPipe, "STEAMINVENTORY_INTERFACE_V001" );
	if ( !steamInventory )
	{
		printf( "steamInventory is null\n" );
		shutdown( 1 );
	}

	ISteamUser017* steamUser = (ISteamUser017*)steamClient->GetISteamUser( hSteamUser, hSteamPipe, STEAMUSER_INTERFACE_VERSION_017 );
	if ( !steamUser )
	{
		printf( "steamUser is null\n" );
		shutdown( 1 );
	}

	signal( SIGINT, ctrl_c_handler );

	clientUser->LogOnWithPassword( false, steamAccountName, steamAccountPassword );

	bool bPlayingGame = false;
	bool bPlayingOnServer = false; // for games that require us to be connected to a server
	uint32 tickCount = 0;

	while ( g_bKeepRunning )
	{
		if ( clientUser->BLoggedOn() && clientFriends->GetPersonaState() != k_EPersonaStateOnline )
		{
			// Spam happens
			// printf( "[*] Going online...\n" );
			clientFriends->SetPersonaState( k_EPersonaStateOnline );
		}

		// process steam user callbacks
		CallbackMsg_t callbackMsg;
		while ( Steam_BGetCallback( hSteamPipe, &callbackMsg ) )
		{
			switch ( callbackMsg.m_iCallback )
			{
				case SteamServersConnected_t::k_iCallback:
				{
					printf( "[*] Logged in\n" );

					// ToDo: reverse ParentalSettings callback
					if ( (*(bool( __thiscall** )(IClientUser*))(*(DWORD*)clientUser + 764))(clientUser) // BIsParentalLockEnabled()
					  && (*(bool( __thiscall** )(IClientUser*))(*(DWORD*)clientUser + 768))(clientUser) ) // BIsParentalLockLocked()
					{
						if ( (*(bool( __thiscall** )(IClientUser*, EParentalFeature))(*(DWORD*)clientUser + 796))(clientUser, k_EParentalFeatureLibrary) // BIsFeatureBlocked()
						  && (*(bool( __thiscall** )(IClientUser*, AppId_t))(*(DWORD*)clientUser + 780))(clientUser, appID) ) // BIsAppBlocked()
						{
							char parentalPinCode[5];
							printf( "[!] Parental Lock Enabled\n" );
							printf( "Enter the Parental Pin code (4 digit): " );
							fgets( parentalPinCode, sizeof( parentalPinCode ), stdin );
							fflush( stdin );
							if ( !(*(bool( __thiscall** )(IClientUser*, const char *))(*(DWORD*)clientUser + 760))(clientUser, parentalPinCode) ) // UnlockParentalLock()
							{
								printf( "[!] UnlockParentalLock - fail\n" );
							}
						}
					}
					break;
				}
				case LicensesUpdated_t::k_iCallback:
				{
					if (appID == 0)
					{
						break;
					}

					if ( (*(bool( __thiscall** )(IClientUser*, AppId_t))(*(DWORD*)clientUser + 692))(clientUser, appID) ) // BIsSubscribedApp
					{
						clientUtils->SetAppIDForCurrentPipe( appID, true );
						bPlayingGame = true;
					}
					else
					{
						printf( "[*] You are not subscribed to this app. Trying to add a free license...\n" );

						SteamAPICall_t hRequestFreeLicenseForApps = (*(SteamAPICall_t( __thiscall** )(IClientBilling*, AppId_t*, int))(*(DWORD*)clientBilling + 24))(clientBilling, &appID, 1); // RequestFreeLicenseForApps
						bool bFailed;
						while ( !clientUtils->IsAPICallCompleted( hRequestFreeLicenseForApps, &bFailed ) )
						{
							Sleep( 100 );
						}

						RequestFreeLicenseResponse_t requestFreeLicenseResponse;
						if ( !clientUtils->GetAPICallResult( hRequestFreeLicenseForApps, &requestFreeLicenseResponse, sizeof( RequestFreeLicenseResponse_t ), RequestFreeLicenseResponse_t::k_iCallback, &bFailed ) )
						{
							printf( "[!] GetAPICallResult failed\n" );
							g_bKeepRunning = false;
							break;
						}
						if ( requestFreeLicenseResponse.m_EResult == k_EResultOK && requestFreeLicenseResponse.m_nGrantedAppIds == 1 )
						{
							printf( "[*] Added a free license\n" );
							clientUtils->SetAppIDForCurrentPipe( appID, true );
							bPlayingGame = true;
						}
						else
						{
							printf( "[!] Failed to add a free license. You do not own this game\n" );
							g_bKeepRunning = false;
							break;
						}
					}

					SetConsoleTextAttribute( hConsole, FOREGROUND_GREEN );
					printf( "Item drop idling is now in progress\n" );
					SetConsoleTextAttribute( hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE );
					break;
				}
				case SteamServerConnectFailure_t::k_iCallback:
				{
					SteamServerConnectFailure_t* steamServerConnectFailure = (SteamServerConnectFailure_t*)callbackMsg.m_pubParam;
					switch ( steamServerConnectFailure->m_eResult )
					{
						case k_EResultInvalidLoginAuthCode:
							printf( "[!] Invalid Steam Guard code\n" );
						case k_EResultAccountLogonDenied:
						{
							char steamGuardCode[33];
							printf( "Enter the Steam Guard code: " );
							scanf( "%32s", steamGuardCode );
							getchar();

							// this is Set2ndFactorAuthCode, however I have to do this because IClientUser.h is outdated
							(*(void( __thiscall** )(IClientUser*, const char*, bool))(*(DWORD*)clientUser + 676))(clientUser, steamGuardCode, false);
							clientUser->LogOnWithPassword( false, steamAccountName, steamAccountPassword );
							break;
						}
						case k_EResultTwoFactorCodeMismatch:
							printf( "[!] Invalid Steam Mobile Authenticator code\n" );
							g_bKeepRunning = false;
							break;
						case k_EResultAccountLogonDeniedNeedTwoFactorCode:
						{
							char steamMobileAuthenticatorCode[33];
							uint8_t secret[20] = {0};
							int ret = getSharedSecret(steamAccountName, secret);

							switch ( ret )
							{
								case 1:
									printf( "Secret file not found! Can not generate 2FA code.\n" );
									break;
								case 2:
									printf( "Secret file is invalid. Can not generate 2FA code.\n" );
									break;
								case 3:
									printf( "Secret is invalid. Can not generate 2FA code.\n" );
									break;
								default:
									get2FACode( secret, steamMobileAuthenticatorCode );
									break;
							}

							if ( ret > 0 )
							{
								printf( "Enter the Steam Mobile Authenticator code: " );
								scanf( "%32s", steamMobileAuthenticatorCode );
								getchar();
							}

							(*(void( __thiscall** )(IClientUser*, const char*))(*(DWORD*)clientUser + 196))(clientUser, steamMobileAuthenticatorCode); // SetTwoFactorCode
							clientUser->LogOnWithPassword( false, steamAccountName, steamAccountPassword );
							break;
						}
						default:
							SetConsoleTextAttribute( hConsole, FOREGROUND_RED );
							printf( "[!] Login failed (%d)\n", steamServerConnectFailure->m_eResult );
							SetConsoleTextAttribute( hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE );
							g_bKeepRunning = false;
							break;
					}

					bPlayingGame = false;
					bPlayingOnServer = false;
					break;
				}
				case SteamServersDisconnected_t::k_iCallback:
				{
					SteamServersDisconnected_t* steamServersDisconnected = (SteamServersDisconnected_t*)callbackMsg.m_pubParam;
					printf( "[!] Disconnected from steam servers (%d)\n", steamServersDisconnected->m_eResult );

					bPlayingGame = false;
					bPlayingOnServer = false;
					break;
				}
				/*
				case GCMessageAvailable_t::k_iCallback:
				{
					GCMessageAvailable_t *GCMessageAvailable = (GCMessageAvailable_t *)callbackMsg.m_pubParam;
					printf( "[*] Game Coordinator message is available (size = %d).\n", GCMessageAvailable->m_nMessageSize);
					break;
				}
				default:
					printf( "[-] User callback: %d (size %d)\n", callbackMsg.m_iCallback, callbackMsg.m_cubParam );
					break;
				*/
			}

			Steam_FreeLastCallback( hSteamPipe );
		}

		// do the actual item drop idling if we're "playing" the game
		if ( bPlayingGame )
		{
			if ( appID == k_nGameIDTF2 )
			{
				static bool bHelloMsgSent = false;
				static bool bGameServerInited = false;

				// do game coordinator stuff
				if ( !bHelloMsgSent )
				{
					unsigned char response[] = {
						0xA6, 0x0F, 0x00, 0x80,      // GC MsgID (k_EMsgGCClientHello)
						0x00, 0x00, 0x00, 0x00,      // header length (0)
						0x08, 0x98, 0xE1, 0xC0, 0x01 // protobuf payload (client version)
					};
					steamGameCoordinator->SendMessage( k_EMsgGCClientHello | k_unGCProtoBufFlag, response, sizeof( response ) );
					printf( "[*] Sent hello msg to game coordinator\n" );

					bHelloMsgSent = true;
				}

				uint32 msgSize;
				while ( steamGameCoordinator->IsMessageAvailable( &msgSize ) )
				{
					uint32 msgType;
					unsigned char* msg = new unsigned char[msgSize];
					EGCResults gcResult = steamGameCoordinator->RetrieveMessage( &msgType, msg, msgSize, &msgSize );
					if ( gcResult == k_EGCResultOK )
					{
						printf( "[*] Retrieved message of type 0x%X from game coordinator (size %d)\n", msgType, msgSize );
						if ( ( msgType & ~k_unGCProtoBufFlag ) == k_EMsgGCClientWelcome )
						{
							printf( "[*] Got welcome msg from game coordinator\n" );
						}
						else if ( ( msgType & ~k_unGCProtoBufFlag ) == k_ESOMsg_CacheSubscriptionCheck )
						{
							unsigned char response[] = {
								0x1C, 0x00, 0x00, 0x80,                              // GC MsgID (k_ESOMsg_CacheSubscriptionRefresh)
								0x00, 0x00, 0x00, 0x00,                              // header length (0)
								0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // protobuf payload (SteamID)
							};
							*(uint64*)(response + 9) = steamUser->GetSteamID().ConvertToUint64(); // make compiler happy
							steamGameCoordinator->SendMessage( k_ESOMsg_CacheSubscriptionRefresh | k_unGCProtoBufFlag, response, sizeof( response ) );
							printf( "[*] Sent response to game coordinator\n" );
						}
						else if ( ( msgType & ~k_unGCProtoBufFlag ) == k_ESOMsg_Create )
						{
							unsigned char* ptr = msg + 8 + 1 + 8 + 1; // skip header & steamid
							int32 type_id = 0;
							ptr += ReadProtoNumber( ptr, &type_id, 4 );
							if (type_id == 1)
							{
								uint64 item_id = 0;
								uint32 def_index = 0;
								ptr += 2 + 1;
								ptr += ReadProtoNumber( ptr, &item_id, 8, false );
								ptr += 1;
								ptr += ReadProtoNumber( ptr, &def_index, 4 ); // skip account_id
								ptr += 1;
								ptr += ReadProtoNumber( ptr, &def_index, 4 ); // skip inventory
								ptr += 1;
								ReadProtoNumber( ptr, &def_index, 4 );

								printf( "NEW ITEM: ItemID %llu, Def %u\n", item_id, def_index );
							}
						}
					}
					else if ( gcResult == k_EGCResultNotLoggedOn )
					{
						// disconnected from steam?
						printf( "[!] GC connection lost\n" );
						bHelloMsgSent = false;
					}
					else
					{
						printf( "[!] Failed to retrieve message from game coordinator\n" );
					}
					delete[] msg;
				}

				// do game server stuff
				static HSteamPipe hSteamGameServerPipe;
				static HSteamUser hSteamGameServerUser;
				static ISteamGameServer012* steamGameServer;
				if ( !bGameServerInited )
				{
					// called by SteamGameServer_Init. needed for games that require us to be connected to a server
					steamClient->SetLocalIPBinding( 0, 26901 );
					hSteamGameServerUser = steamClient->CreateLocalUser( &hSteamGameServerPipe, k_EAccountTypeGameServer );
					if ( !hSteamGameServerPipe || !hSteamGameServerUser )
					{
						printf( "CreateLocalUser failed (2)\n" );
						break;
					}

					steamGameServer = (ISteamGameServer012*)steamClient->GetISteamGameServer( hSteamGameServerUser, hSteamGameServerPipe, STEAMGAMESERVER_INTERFACE_VERSION_012 );
					if ( !steamGameServer )
					{
						printf( "steamGameServer is null\n" );
						steamClient->ReleaseUser( hSteamGameServerPipe, hSteamGameServerUser );
						steamClient->BReleaseSteamPipe( hSteamGameServerPipe );
						break;
					}

					steamGameServer->InitGameServer( 0, 27015, MASTERSERVERUPDATERPORT_USEGAMESOCKETSHARE, k_unServerFlagSecure, k_nGameIDTF2, "3158168" );
					steamGameServer->SetProduct( "tf" );
					steamGameServer->SetGameDescription( "Team Fortress" );
					steamGameServer->SetModDir( "tf" );
					steamGameServer->SetDedicatedServer( false );
					steamGameServer->LogOnAnonymous();
					steamGameServer->SetMaxPlayerCount( 1 );
					steamGameServer->SetBotPlayerCount( 0 );
					steamGameServer->SetPasswordProtected( true );
					steamGameServer->SetRegion( "-1" );
					steamGameServer->SetServerName( "Team Fortress 2" );
					steamGameServer->SetMapName( "ctf_2fort" );
					steamGameServer->SetGameData( "tf_mm_trusted:0,tf_mm_servermode:0,lobby:0,steamblocking:0" );
					steamGameServer->SetKeyValue( "tf_gamemode_ctf", "1" );
					steamGameServer->SetKeyValue( "sv_tags", "ctf" );
					steamGameServer->SetGameTags( "ctf" );

					bGameServerInited = true;
				}

				if ( !bPlayingOnServer )
				{
					static HAuthTicket hAuthTicket = 0;
					if ( hAuthTicket )
					{
						steamUser->CancelAuthTicket( hAuthTicket );
						steamGameServer->EndAuthSession( steamUser->GetSteamID() );
						hAuthTicket = 0;
					}

					unsigned char ticket[1024];
					uint32 ticketSize;
					hAuthTicket = steamUser->GetAuthSessionTicket( ticket, sizeof( ticket ), &ticketSize );
					if ( hAuthTicket != k_HAuthTicketInvalid )
					{
						EBeginAuthSessionResult beginAuthSessionResult = steamGameServer->BeginAuthSession( ticket, ticketSize, steamUser->GetSteamID() );
						if ( beginAuthSessionResult == k_EBeginAuthSessionResultOK )
						{
							bPlayingOnServer = true;
						}
						else
						{
							printf( "[!] BeginAuthSession failed (%d)\n", beginAuthSessionResult );
						}
					}
					else
					{
						printf( "[!] GetAuthSessionTicket failed\n" );
					}
				}

				// process steam game server callbacks
				while ( Steam_BGetCallback( hSteamGameServerPipe, &callbackMsg ) )
				{
					switch ( callbackMsg.m_iCallback )
					{
						case ValidateAuthTicketResponse_t::k_iCallback:
						{
							ValidateAuthTicketResponse_t* validateAuthTicketResponse = (ValidateAuthTicketResponse_t*)callbackMsg.m_pubParam;
							if ( validateAuthTicketResponse->m_eAuthSessionResponse == k_EAuthSessionResponseOK )
							{
								printf( "[*] BeginAuthSession callback ok\n" );
								//steamGameServer->BUpdateUserData( validateAuthTicketResponse->m_SteamID, "Player", 0 );
							}
							else
							{
								printf( "[!] BeginAuthSession callback failed (%d)\n", validateAuthTicketResponse->m_eAuthSessionResponse );
								bPlayingOnServer = false;
							}
							break;
						}
						/*default:
							printf( "Game server callback: %d\n", callbackMsg.m_iCallback );
							break;*/
					}

					Steam_FreeLastCallback( hSteamGameServerPipe );
				}

				if ( !g_bKeepRunning )
				{
					steamGameServer->LogOff();
					steamClient->ReleaseUser( hSteamGameServerPipe, hSteamGameServerUser );
					steamClient->BReleaseSteamPipe( hSteamGameServerPipe );
					break;
				}
			}
			else
			{
				// SendHeartbeat is safe to call on every frame since the API is internally rate-limited.
				steamInventory->SendItemDropHeartbeat();

				// Do not spam
				if ( tickCount == 0 )
				{
					SteamInventoryResult_t steamInventoryResult;
					if ( steamInventory->TriggerItemDrop( &steamInventoryResult, dropListDefinition ) )
					{
						while ( steamInventory->GetResultStatus(steamInventoryResult) == k_EResultPending )
						{
							Sleep( 100 );
						}

						uint32 itemsCount = 0;
						if ( steamInventory->GetResultItems( steamInventoryResult, NULL, &itemsCount ) && (itemsCount > 0) )
						{
							SteamItemDetails_t *details = new SteamItemDetails_t[itemsCount];
							if ( steamInventory->GetResultItems( steamInventoryResult, details, &itemsCount ) )
							{
								for ( uint32 idx = 0; idx < itemsCount; ++idx )
								{
									printf( "NEW ITEM: ItemID %llu, Def %d\n", details[idx].m_itemId, details[idx].m_iDefinition );
								}
							}
							delete[] details;
						}
					}

					steamInventory->DestroyResult( steamInventoryResult );
				}

				tickCount = ( tickCount + 1 ) % 10;
			}
		}

		Sleep( 1000 );
	}

	if ( clientUser->BLoggedOn() )
	{
		clientFriends->SetPersonaState( k_EPersonaStateOffline );
		clientUser->LogOff();
		printf( "[*] Logged off\n" );
	}

	clientEngine->ReleaseUser( hSteamPipe, hSteamUser );
	clientEngine->BReleaseSteamPipe( hSteamPipe );
	clientEngine->BShutdownIfAllPipesClosed();

	printf( "Press enter to exit...\n" );
	getchar();
	return 0;
}
