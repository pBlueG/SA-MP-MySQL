#include     <a_samp>

// Change MAX_PLAYERS to the amount of players (slots) you want
// It is by default 1000 (as of 0.3.7 version)
#undef      MAX_PLAYERS
#define     MAX_PLAYERS          50

#include     <a_mysql>

// MySQL configuration
#define     MYSQL_HOST          "127.0.0.1"
#define     MYSQL_USER          "username"
#define     MYSQL_PASSWORD      "password"
#define     MYSQL_DATABASE      "database"

// How many seconds until it kicks the player for taking too long to login
#define     SECONDS_TO_LOGIN     30

// MySQL connection handle
new MySQL: g_SQL;

// player data
enum E_PLAYERS
{
    ORM:ORM_ID,
    
    ID,
    Name[MAX_PLAYER_NAME],
    Password[65], // The output of SHA256_PassHash function (which was added in 0.3.7 R1 version) is always 256 bytes in length, or the equivalent of 64 Pawn cells
    Salt[17],
    Kills,
    Deaths,

    bool: IsLoggedIn,
    LoginAttempts,
    LoginTimer
};
new Player[MAX_PLAYERS][E_PLAYERS];

new g_MysqlRaceCheck[MAX_PLAYERS];

//dialog data
enum
{
    DIALOG_UNUSED,

    DIALOG_LOGIN,
    DIALOG_REGISTER
};

main() {}

public OnGameModeInit()
{
    new MySQLOpt: option_id = mysql_init_options();

    mysql_set_option(option_id, AUTO_RECONNECT, true); // It automatically reconnects when loosing connection to mysql server
    mysql_set_option(option_id, MULTI_STATEMENTS, true); // Allows to execute multiple queries at once

    g_SQL = mysql_connect(MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DATABASE, option_id); // AUTO_RECONNECT and MULTI_STATEMENTS are enabled for this connection handle only
    if (g_SQL == MYSQL_INVALID_HANDLE || mysql_errno(g_SQL) != 0)
    {
        print("MySQL connection failed. Server is shutting down.");
        SendRconCommand("exit"); // Close the server if there is no connection
        return 1;
    }

    print("MySQL connection is successful.");
    SetupPlayerTable();
    // If the table has been created, the "SetupPlayerTable" function does not have any purpose so you may remove it completely
    return 1;
}

public OnGameModeExit()
{
    // save all player data before closing connection
    for (new i = 0, j = GetPlayerPoolSize(); i <= j; i++) // GetPlayerPoolSize function was added in 0.3.7 version and gets the highest playerid currently in use on the server
    {
        if (IsPlayerConnected(i) && Player[i][IsLoggedIn] == true)
        {
            orm_save(Player[i][ORM_ID]);
            orm_destroy(Player[i][ORM_ID]);
            Player[i][IsLoggedIn] = false;
        }
    }

    mysql_close(g_SQL);
    return 1;
}

public OnPlayerConnect(playerid)
{
    g_MysqlRaceCheck[playerid]++;

    // reset player data
    for (new E_PLAYERS: e; e < E_PLAYERS; ++e)
    {
        Player[playerid][e] = 0;
    }

    GetPlayerName(playerid, Player[playerid][Name], MAX_PLAYER_NAME);

    // create orm instance and register all needed variables
    new ORM: ormid = Player[playerid][ORM_ID] = orm_create("players", g_SQL);

    orm_addvar_int(ormid, Player[playerid][ID], "id");
    orm_addvar_string(ormid, Player[playerid][Name], MAX_PLAYER_NAME, "username");
    orm_addvar_string(ormid, Player[playerid][Password], 65, "password");
    orm_addvar_string(ormid, Player[playerid][Salt], 17, "salt");
    orm_addvar_int(ormid, Player[playerid][Kills], "kills");
    orm_addvar_int(ormid, Player[playerid][Deaths], "deaths");
    orm_setkey(ormid, "username");

    // tell the orm system to load all data, assign it to our variables and call our callback when ready
    orm_load(ormid, "OnPlayerDataLoaded", "dd", playerid, g_MysqlRaceCheck[playerid]);
    return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
    g_MysqlRaceCheck[playerid]++;

    // sends an UPDATE query
    if (Player[playerid][IsLoggedIn] == true)
    {
        orm_save(Player[playerid][ORM_ID]);
        orm_destroy(Player[playerid][ORM_ID]);
    }

    KillTimer(Player[playerid][LoginTimer]);
    Player[playerid][LoginTimer] = 0;
    Player[playerid][IsLoggedIn] = false;
    // sets "IsLoggedIn" to false when the player disconnects, it prevents from saving the player data twice when "gmx" is used
    return 1;
}

public OnPlayerSpawn(playerid)
{
    // Spawn them in Las Venturas (The High Roller)
    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, 0);

    SetPlayerPos(playerid, 1958.3783, 1343.1572, 15.3746);
    SetPlayerFacingAngle(playerid, 270.1425);
    return 1;
}

public OnPlayerDeath(playerid, killerid, reason)
{
    // increase the deaths by 1 of the player who died
    Player[playerid][Deaths]++;

    // if the killer is a valid (connected) player, increase their kills by 1
    if (killerid != INVALID_PLAYER_ID)
    {
        Player[killerid][Kills]++;
    }
    return 1;
}

public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    switch (dialogid)
    {
        case DIALOG_UNUSED: return 1; // Useful for dialogs that contain only information and we do nothing depending on whether they responded or not

        case DIALOG_LOGIN:
        {
            if (!response) return Kick(playerid);

            new hashed_pass[65];
            SHA256_PassHash(inputtext, Player[playerid][Salt], hashed_pass, 65);

            if (strcmp(hashed_pass, Player[playerid][Password]) == 0)
            {
                //correct password, spawn the player
                ShowPlayerDialog(playerid, DIALOG_UNUSED, DIALOG_STYLE_MSGBOX, "Login", "You have been successfully logged in.", "Okay", "");

             	KillTimer(Player[playerid][LoginTimer]);
             	Player[playerid][LoginTimer] = 0;
                Player[playerid][IsLoggedIn] = true;

                SetSpawnInfo(playerid, NO_TEAM, 0, 1958.3783, 1343.1572, 15.3746, 270.1425, 0, 0, 0, 0, 0, 0);
                SpawnPlayer(playerid);
            }
            else
            {
                Player[playerid][LoginAttempts]++;

                if (Player[playerid][LoginAttempts] >= 3)
                {
                    ShowPlayerDialog(playerid, DIALOG_UNUSED, DIALOG_STYLE_MSGBOX, "Login", "You have mistyped your password too often (3 times).", "Okay", "");
                    DelayedKick(playerid);
                }
                else ShowPlayerDialog(playerid, DIALOG_LOGIN, DIALOG_STYLE_PASSWORD, "Login", "Wrong password!\nPlease enter your password in the field below:", "Login", "Abort");
            }
        }
        case DIALOG_REGISTER:
        {
            if (!response) return Kick(playerid);

            if (strlen(inputtext) <= 5) return ShowPlayerDialog(playerid, DIALOG_REGISTER, DIALOG_STYLE_PASSWORD, "Registration", "Your password must be longer than 5 characters!\nPlease enter your password in the field below:", "Register", "Abort");

            // 16 random characters from 33 to 126 (in ASCII) for the salt
            for (new i = 0; i < 16; i++) Player[playerid][Salt][i] = random(94) + 33;
            SHA256_PassHash(inputtext, Player[playerid][Salt], Player[playerid][Password], 65);

			// sends an INSERT query
            orm_save(Player[playerid][ORM_ID], "OnPlayerRegister", "d", playerid);
        }

        default: return 0; // dialog ID was not found, search in other scripts
    }
    return 1;
}

//-----------------------------------------------------

forward OnPlayerDataLoaded(playerid, race_check);
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
    if (race_check != g_MysqlRaceCheck[playerid]) return Kick(playerid);

    orm_setkey(Player[playerid][ORM_ID], "id");

    new string[115];
    switch (orm_errno(Player[playerid][ORM_ID]))
    {
    case ERROR_OK:
    {
            format(string, sizeof string, "This account (%s) is registered. Please login by entering your password in the field below:", Player[playerid][Name]);
            ShowPlayerDialog(playerid, DIALOG_LOGIN, DIALOG_STYLE_PASSWORD, "Login", string, "Login", "Abort");
			
            // from now on, the player has 30 seconds to login
            Player[playerid][LoginTimer] = SetTimerEx("OnLoginTimeout", SECONDS_TO_LOGIN * 1000, false, "d", playerid);
        }
        case ERROR_NO_DATA:
        {
            format(string, sizeof string, "Welcome %s, you can register by entering your password in the field below:", Player[playerid][Name]);
            ShowPlayerDialog(playerid, DIALOG_REGISTER, DIALOG_STYLE_PASSWORD, "Registration", string, "Register", "Abort");
        }
    }
    return 1;
}

forward OnLoginTimeout(playerid);
public OnLoginTimeout(playerid)
{
    ShowPlayerDialog(playerid, DIALOG_UNUSED, DIALOG_STYLE_MSGBOX, "Login", "You have been kicked for taking too long to login successfully to your account.", "Okay", "");
    DelayedKick(playerid);
    return 1;
}

forward OnPlayerRegister(playerid);
public OnPlayerRegister(playerid)
{
    ShowPlayerDialog(playerid, DIALOG_UNUSED, DIALOG_STYLE_MSGBOX, "Registration", "Account successfully registered, you have been automatically logged in.", "Okay", "");

    Player[playerid][IsLoggedIn] = true;

    SetSpawnInfo(playerid, NO_TEAM, 0, 1958.3783, 1343.1572, 15.3746, 270.1425, 0, 0, 0, 0, 0, 0);
    SpawnPlayer(playerid);
    return 1;
}

forward _KickPlayerDelayed(playerid);
public _KickPlayerDelayed(playerid)
{
    Kick(playerid);
    return 1;
}


//-----------------------------------------------------

DelayedKick(playerid, time = 500)
{
    SetTimerEx("_KickPlayerDelayed", time, false, "d", playerid);
    return 1;
}

SetupPlayerTable()
{
    mysql_tquery(g_SQL, "CREATE TABLE IF NOT EXISTS `players` (`id` int(11) NOT NULL AUTO_INCREMENT, `username` varchar(24) NOT NULL, `password` char(64) NOT NULL, `salt` char(16) NOT NULL, `kills` int(11) NOT NULL DEFAULT '0', `deaths` int(11) NOT NULL DEFAULT '0', PRIMARY KEY (`id`), UNIQUE KEY `username` (`username`))");
    return 1;
}
