#include "stdafx.h"
#include "token_generator/token_generator.h"

#define SIDI_NAME "Steam Item Drop Idler"
#define SIDI_VERSION "2.03"

CSteamAPILoader g_steamAPILoader;

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

	clientUser->LogOnWithPassword( false, steamAccountName, steamAccountPassword );

	bool bPlayingGame = false;
	bool bPlayingOnServer = false; // for games that require us to be connected to a server
	while ( true )
	{
		// process steam user callbacks
		CallbackMsg_t callbackMsg;
		while ( Steam_BGetCallback( hSteamPipe, &callbackMsg ) )
		{
			switch ( callbackMsg.m_iCallback )
			{
				case SteamServersConnected_t::k_iCallback:
				{
					clientFriends->SetPersonaState( k_EPersonaStateOnline );

					if ( (*(bool( __thiscall** )(IClientUser*, AppId_t))(*(DWORD*)clientUser + 692))(clientUser, appID) ) // BIsSubscribedApp
					{
						clientUtils->SetAppIDForCurrentPipe( appID, true );
						bPlayingGame = true;
					}
					else
					{
						printf( "You are not subscribed to this app. Trying to add a free license...\n" );

						SteamAPICall_t hRequestFreeLicenseForApps = (*(SteamAPICall_t( __thiscall** )(IClientBilling*, AppId_t*, int))(*(DWORD*)clientBilling + 24))(clientBilling, &appID, 1); // RequestFreeLicenseForApps
						bool bFailed;
						while ( !clientUtils->IsAPICallCompleted( hRequestFreeLicenseForApps, &bFailed ) )
						{
							Sleep( 100 );
						}

						RequestFreeLicenseResponse_t requestFreeLicenseResponse;
						if ( !clientUtils->GetAPICallResult( hRequestFreeLicenseForApps, &requestFreeLicenseResponse, sizeof( RequestFreeLicenseResponse_t ), RequestFreeLicenseResponse_t::k_iCallback, &bFailed ) )
						{
							printf( "GetAPICallResult failed\n" );
							shutdown( 1 );
						}
						if ( requestFreeLicenseResponse.m_EResult == k_EResultOK && requestFreeLicenseResponse.m_nGrantedAppIds == 1 )
						{
							printf( "Added a free license\n" );
							clientUtils->SetAppIDForCurrentPipe( appID, true );
							bPlayingGame = true;
						}
						else
						{
							printf( "Failed to add a free license. You do not own this game\n" );
							shutdown( 1 );
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
							printf( "Invalid Steam Guard code\n" );
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
							printf( "Invalid Steam Mobile Authenticator code\n" );
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
							printf( "Login failed (%d)\n", steamServerConnectFailure->m_eResult );
							SetConsoleTextAttribute( hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE );
							break;
					}

					bPlayingGame = false;
					bPlayingOnServer = false;
					break;
				}
				case SteamServersDisconnected_t::k_iCallback:
				{
					SteamServersDisconnected_t* steamServersDisconnected = (SteamServersDisconnected_t*)callbackMsg.m_pubParam;
					printf( "Disconnected from steam servers (%d)\n", steamServersDisconnected->m_eResult );

					bPlayingGame = false;
					bPlayingOnServer = false;
					break;
				}
				/*default:
					printf( "User callback: %d\n", callbackMsg.m_iCallback );
					break;*/
			}

			Steam_FreeLastCallback( hSteamPipe );
		}

		// do the actual item drop idling if we're "playing" the game
		if ( bPlayingGame )
		{
			if ( appID == 440 )
			{
				static bool bHelloMsgSent = false;
				static bool bGameServerInited = false;

				// do game coordinator stuff
				if ( !bHelloMsgSent )
				{
					// k_EMsgGCClientHello
					unsigned char response[] = { 0xA6, 0x0F, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x08, 0x98, 0xE1, 0xC0, 0x01 };
					steamGameCoordinator->SendMessage( 0x80000FA6, response, sizeof( response ) );
					printf( "Sent hello msg to game coordinator\n" );

					bHelloMsgSent = true;
				}

				uint32 msgSize;
				while ( steamGameCoordinator->IsMessageAvailable( &msgSize ) )
				{
					uint32 msgType;
					unsigned char* msg = new unsigned char[msgSize];
					if ( steamGameCoordinator->RetrieveMessage( &msgType, msg, msgSize, &msgSize ) == k_EGCResultOK )
					{
						printf( "Retrieved message of type 0x%X from game coordinator\n", msgType );
						if ( msgType == 0x80000FA4 ) // k_EMsgGCClientWelcome
						{
							printf( "Got welcome msg from game coordinator\n" );
						}
						else if ( msgType == 0x8000001B ) // k_ESOMsg_CacheSubscriptionCheck
						{
							// k_ESOMsg_CacheSubscriptionRefresh
							unsigned char response[] = { 0x1C, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
							*(CSteamID*)&response[9] = steamUser->GetSteamID();
							steamGameCoordinator->SendMessage( 0x8000001C, response, sizeof( response ) );
							printf( "Sent response to game coordinator\n" );
						}
					}
					else
					{
						printf( "Failed to retrieve message from game coordinator\n" );
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
						shutdown( 1 );
					}

					steamGameServer = (ISteamGameServer012*)steamClient->GetISteamGameServer( hSteamGameServerUser, hSteamGameServerPipe, STEAMGAMESERVER_INTERFACE_VERSION_012 );
					if ( !steamGameServer )
					{
						printf( "steamGameServer is null\n" );
						shutdown( 1 );
					}

					steamGameServer->InitGameServer( 0, 27015, MASTERSERVERUPDATERPORT_USEGAMESOCKETSHARE, k_unServerFlagSecure, 440, "3158168" );
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
					//steamGameServer->EnableHeartbeats( true );

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
							printf( "BeginAuthSession failed (%d)\n", beginAuthSessionResult );
						}
					}
					else
					{
						printf( "GetAuthSessionTicket failed\n" );
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
								printf( "BeginAuthSession callback ok\n" );
								//steamGameServer->BUpdateUserData( validateAuthTicketResponse->m_SteamID, "Player", 0 );
							}
							else
							{
								printf( "BeginAuthSession callback failed (%d)\n", validateAuthTicketResponse->m_eAuthSessionResponse );
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
			}
			else
			{
				steamInventory->SendItemDropHeartbeat();

				SteamInventoryResult_t steamInventoryResult;
				steamInventory->TriggerItemDrop( &steamInventoryResult, dropListDefinition );
				steamInventory->DestroyResult( steamInventoryResult );
			}
		}

		Sleep( 1000 );
	}

	return 0;
}
