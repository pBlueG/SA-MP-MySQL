#include <a_samp>
#include <a_mysql>

native WP_Hash(buffer[], len, const str[]);

#define WHITE 0xFFFFFFAA
#define GREY 0xAFAFAFAA
#define RED 0xFF0000AA
#define YELLOW 0xFFFF00AA
#define LIGHTBLUE 0x33CCFFAA

#define CHAT_WHITE "{FFFFFF}"
#define CHAT_GREY "{AFAFAF}"
#define CHAT_RED "{FF0000}"
#define CHAT_YELLOW "{FFFF00}"
#define CHAT_LIGHTBLUE "{33CCFF}"

forward OnPlayerDataLoaded(playerid, race_check);
forward OnPlayerRegister(playerid);


//MySQL configuration
#define SQL_HOST "127.0.0.1"
#define SQL_DB "database"
#define SQL_USER "username"
#define SQL_PASS "password"

//MySQL connection handle
new SQL = -1;

//player data
enum E_PLAYERS
{
	ID,
	Name[MAX_PLAYER_NAME],
	Password[129],
	Money,
	
	bool:IsLoggedIn,
	bool:IsRegistered,
	LoginAttempts,
	LoginTimer
};
new Player[MAX_PLAYERS][E_PLAYERS];
new MysqlRaceCheck[MAX_PLAYERS];

//dialog data
enum {
	DIALOG_UNUSED,
	
	DIALOG_LOGIN,
	DIALOG_REGISTER,
};

public OnGameModeInit()
{
	mysql_log(LOG_ERROR | LOG_WARNING, LOG_TYPE_HTML); //logs errors and warnings into a nice .html log file
	SQL = mysql_connect(SQL_HOST, SQL_USER, SQL_DB, SQL_PASS);
	
	SetupPlayerTable();
	return 1;
}

public OnGameModeExit()
{
	//save all player data before closing connection
	for(new p=0; p < MAX_PLAYERS; ++p)
		if(IsPlayerConnected(p))
		    UpdatePlayerData(p);

	mysql_close();
	return 1;
}

public OnPlayerConnect(playerid)
{
	MysqlRaceCheck[playerid]++;
	//reset player data
	for(new E_PLAYERS:e; e < E_PLAYERS; ++e)
	    Player[playerid][e] = 0;

	new query[128];
	GetPlayerName(playerid, Player[playerid][Name], MAX_PLAYER_NAME);

	//send a query to recieve all the stored player data from the table
	mysql_format(SQL, query, sizeof(query), "SELECT * FROM `players` WHERE `username` = '%e' LIMIT 1", Player[playerid][Name]);
	mysql_tquery(SQL, query, "OnPlayerDataLoaded", "dd", playerid, MysqlRaceCheck[playerid]);
	return 1;
}

public OnPlayerDataLoaded(playerid, race_check)
{
	/*	race condition check:
		player A connects -> SELECT query is fired -> this query takes very long
		while the query is still processing, player A with playerid 2 disconnects
		player B joins now with playerid 2 -> our laggy SELECT query is finally finished, but for the wrong player
		
		what do we do against it?
		we create a connection count for each playerid and increase it everytime the playerid connects or disconnects
		we also pass the current value of the connection count to our OnPlayerDataLoaded callback
		then we check if current connection count is the same as connection count we passed to the callback
		if yes, everything is okay, if not, we just kick the player
	*/
	if(race_check != MysqlRaceCheck[playerid])
	    return Kick(playerid);
	
	
    new string[128];
	if(cache_num_rows() > 0)
	{
        AssignPlayerData(playerid);
        
		format(string, sizeof(string), CHAT_WHITE "This account (" CHAT_YELLOW "%s" CHAT_WHITE ") is registered. Please login by entering your password in the field below:", Player[playerid][Name]);
		ShowPlayerDialog(playerid, DIALOG_LOGIN, DIALOG_STYLE_PASSWORD, "Login", string, "Login", "Abort");
		Player[playerid][IsRegistered] = true;
	}
	else
	{
		format(string, sizeof(string), CHAT_WHITE "Welcome " CHAT_YELLOW "%s" CHAT_WHITE ", you can register by entering your password in the field below:", Player[playerid][Name]);
		ShowPlayerDialog(playerid, DIALOG_REGISTER, DIALOG_STYLE_PASSWORD, "Registration", string, "Register", "Abort");
		Player[playerid][IsRegistered] = false;
	}
	return 1;
}

public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
	switch(dialogid)
	{
	    case DIALOG_LOGIN:
	    {
	        if(!response)
	            return Kick(playerid);

			if(strlen(inputtext) <= 5)
				return ShowPlayerDialog(playerid, DIALOG_LOGIN, DIALOG_STYLE_PASSWORD, "Login",
					CHAT_RED "Your password must be longer than 5 characters!\n" CHAT_WHITE "Please enter your password in the field below:",
					"Login", "Abort");

			new hashed_pass[129];
			WP_Hash(hashed_pass, sizeof(hashed_pass), inputtext);
			
			if(strcmp(hashed_pass, Player[playerid][Password]) == 0)
			{
				//correct password, spawn the player
				ShowPlayerDialog(playerid, DIALOG_UNUSED, DIALOG_STYLE_MSGBOX, "Login", "You have been successfully logged in.", "Okay", "");
				Player[playerid][IsLoggedIn] = true;
				
			    SetSpawnInfo(playerid, 0, 0, 0.0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0);
				SpawnPlayer(playerid);
			}
			else
			{
			    Player[playerid][LoginAttempts]++;
			    if(Player[playerid][LoginAttempts] >= 3)
			    {
					ShowPlayerDialog(playerid, DIALOG_UNUSED, DIALOG_STYLE_MSGBOX, "Login", CHAT_RED "You have mistyped your password too often (3 times).", "Okay", "");
			        DelayedKick(playerid);
				}
				else
				    ShowPlayerDialog(playerid, DIALOG_LOGIN, DIALOG_STYLE_PASSWORD, "Login", CHAT_RED "Wrong password!\n" CHAT_WHITE "Please enter your password in the field below:", "Login", "Abort");
			}
	    }
	    
	    case DIALOG_REGISTER:
	    {
	        if(!response)
	            return Kick(playerid);
	            
            if(strlen(inputtext) <= 5)
				return ShowPlayerDialog(playerid, DIALOG_REGISTER, DIALOG_STYLE_PASSWORD, "Registration",
					CHAT_RED "Your password must be longer than 5 characters!\n" CHAT_WHITE "Please enter your password in the field below:",
					"Register", "Abort");
					
			new query[256];
			WP_Hash(Player[playerid][Password], 129, inputtext);
			mysql_format(SQL, query, sizeof(query), "INSERT INTO `players` (`username`, `password`) VALUES ('%e', '%s')", Player[playerid][Name], Player[playerid][Password]);
			mysql_tquery(SQL, query, "OnPlayerRegister", "d", playerid);
	    }
	    
	    default:
			return 0;
	}
	return 1;
}

public OnPlayerRegister(playerid)
{
	Player[playerid][ID] = cache_insert_id();
	
	ShowPlayerDialog(playerid, DIALOG_UNUSED, DIALOG_STYLE_MSGBOX, "Registration", "Account successfully registered, you have been automatically logged in.", "Okay", "");
	Player[playerid][IsLoggedIn] = true;
	Player[playerid][IsRegistered] = true;
	
    SetSpawnInfo(playerid, 0, 0, 0.0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0);
	SpawnPlayer(playerid);
	return 1;
}

public OnPlayerSpawn(playerid)
{
	ResetPlayerMoney(playerid);
	GivePlayerMoney(playerid, Player[playerid][Money]);
	
	return 1;
}

public OnPlayerDisconnect(playerid,reason)
{
    MysqlRaceCheck[playerid]++;
	UpdatePlayerData(playerid);
	return 1;
}


stock AssignPlayerData(playerid)
{
	Player[playerid][ID] = cache_get_field_content_int(0, "id");
	cache_get_field_content(0, "password", Player[playerid][Password], SQL, 129);
	Player[playerid][Money] = cache_get_field_content_int(0, "money");
	return 1;
}

stock UpdatePlayerData(playerid)
{
	if(Player[playerid][IsLoggedIn] == false)
	    return 0;
	    
	new query[256];
	mysql_format(SQL, query, sizeof(query), "UPDATE `players` SET `money` = '%d' WHERE `id` = '%d' LIMIT 1", Player[playerid][Money], Player[playerid][ID]);
	mysql_tquery(SQL, query, "", "");
	return 1;
}

stock SetupPlayerTable()
{
	mysql_query(SQL, "CREATE TABLE IF NOT EXISTS `players` (`id` int(11) NOT NULL auto_increment PRIMARY KEY,`username` varchar(30) NOT NULL,`password` varchar(130) NOT NULL,`money` int(10) NOT NULL default '0')", false);
	return 1;
}


stock DelayedKick(playerid, time=500)
{
	SetTimerEx("_KickPlayerDelayed", time, false, "d", playerid);
	return 1;
}

forward _KickPlayerDelayed(playerid);
public _KickPlayerDelayed(playerid)
{
	Kick(playerid);
	return 1;
}

main() {}

