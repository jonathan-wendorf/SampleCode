#include "DatabaseManager.h"
#include <iostream>

// A database manager I created for a game networks class.  This class manages the creation and saving of usernames, passwords, and high scores from playing the game Swarch.
// This class uses the SQLite API to construct a simple database for use on a game server, and can return username and high score data for display on a web server

DatabaseManager::DatabaseManager(void)
{
	error = sqlite3_open("Swarch.db", &db);

	if(error)
	{
		std::cout << "Couldn't open the database, closing the database" << std::endl;
		sqlite3_close(db);
		dbOpened = false;
	}
	else
		dbOpened = true;
}

DatabaseManager::~DatabaseManager(void)
{
	sqlite3_close(db);
}

bool DatabaseManager::doesUserExistInDB(std::string userName)
{
	std::string query = "SELECT COUNT(passWord) FROM swarchTable WHERE userName = ?";
	
	// Prepare the query for use by the sqlite
	error = sqlite3_prepare_v2(db, query.c_str(), query.length(), &stmt, &tail);

	sqlite3_bind_text(stmt, 1, userName.c_str(), userName.length(), SQLITE_TRANSIENT);

	if(error != SQLITE_OK)
	{
		std::cout << "Couldn't prepare SQL query" << std::endl;
		return false;
	}

	// Run the query
	if(sqlite3_step(stmt) == SQLITE_ROW)
	{
		// Collect the count of how many passwords exist under the username
		int count = sqlite3_column_int(stmt, 0);
		std::cout << "Number of users matching this name: " << count << std::endl;

		// If there is a password, return true
		if(count > 0)
		{
			std::cout << "They match!" << std::endl;
			sqlite3_finalize(stmt);
			return true;
		}
	}

	std::cout << "No match could be found" << std::endl;
	
	// sqlite3_finalize() frees up our SQL statement that we set up in sqlite3_prepare_v2()
	sqlite3_finalize(stmt);
	return false;
}

bool DatabaseManager::doesPasswordMatchUser(std::string userName, std::string passWord)
{
	std::string query = "SELECT passWord FROM swarchTable WHERE userName = ?";

	// Prepare the query for use by the sqlite
	error = sqlite3_prepare_v2(db, query.c_str(), query.length(), &stmt, &tail);

	sqlite3_bind_text(stmt, 1, userName.c_str(), userName.length(), SQLITE_TRANSIENT);

	if(error != SQLITE_OK)
	{
		std::cout << "Couldn't prepare SQL query" << std::endl;
		return false;
	}

	// sqlite3_step() runs our query on the database
	if(sqlite3_step(stmt) == SQLITE_ROW)
	{
		// Acquires the password from the database (which we recast from a const char* to a string)
		std::string password = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
		
		if(password.compare(passWord) == 0)
		{
			std::cout << "They match!" << std::endl;
			sqlite3_finalize(stmt);
			return true;
		}
	}

	std::cout << "No match" << std::endl;

	// sqlite3_finalize() frees up our SQL statement that we set up in sqlite3_prepare_v2()
	sqlite3_finalize(stmt);
	return false;
}

void DatabaseManager::updateEntry(std::string userName, int score)
{
	std::string query = "UPDATE swarchTable SET score = ? WHERE userName = ? AND score < ?";

	// Prepare the query for use by the sqlite
	error = sqlite3_prepare_v2(db, query.c_str(), query.length(), &stmt, &tail);

	sqlite3_bind_int(stmt, 1, score);
	sqlite3_bind_text(stmt, 2, userName.c_str(), userName.length(), SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 3, score);

	if(error != SQLITE_OK)
	{
		std::cout << "Couldn't prepare SQL query" << std::endl;
		sqlite3_finalize(stmt);
		return;
	}

	// Run the SQL query
	if(sqlite3_step(stmt) == SQLITE_ERROR)
	{
		std::cout << "Entry not updated" << std::endl;
		sqlite3_finalize(stmt);
		return;
	}

	std::cout << "Entry updated" << std::endl;

	// sqlite3_finalize() frees up our SQL statement that we set up in sqlite3_prepare_v2()
	sqlite3_finalize(stmt);
}

void DatabaseManager::insertEntry(std::string userName, std::string passWord)
{
	std::string query = "INSERT INTO swarchTABLE VALUES (?, ?, 0)";

	// Prepares the SQL Query for use
	error = sqlite3_prepare_v2(db, query.c_str(), query.length(), &stmt, &tail);

	// Binds info to the '?'
	sqlite3_bind_text(stmt, 1, userName.c_str(), userName.length(), SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, passWord.c_str(), passWord.length(), SQLITE_TRANSIENT);

	// If the SQL file wasn't prepared correctly, exit the method
	if(error != SQLITE_OK)
	{
		std::cout << "Couldn't prepare sql" << std::endl;
		sqlite3_finalize(stmt);
		return;
	}

	// Run our SQL program
	if(sqlite3_step(stmt) == SQLITE_ERROR)
	{
		std::cout << "Entry not added" << std::endl;
		sqlite3_finalize(stmt);
		return;
	}

	std::cout << "Entry added" << std::endl;

	// sqlite3_finalize() frees up our SQL statement that we set up in sqlite3_prepare_v2()
	sqlite3_finalize(stmt);
}

bool DatabaseManager::doesTableExistInDB()
{
	// Construct a query that will insert a new user into our database
	// The query we are using inserts a new entry based on username, password and score
	std::string query = "SELECT COUNT(*) FROM swarchTable";

	// Prepares the SQL Query for use
	error = sqlite3_prepare_v2(db, query.c_str(), query.length(), &stmt, &tail);

	// If the SQL file wasn't prepared correctly, exit the method
	if(error != SQLITE_OK)
	{
		std::cout << "Couldn't prepare sql" << std::endl;
		sqlite3_finalize(stmt);
		return false;
	}

	// Run our SQL program
	if(sqlite3_step(stmt) == SQLITE_ROW)
	{
		int count = sqlite3_column_int(stmt, 0);
		std::cout << count << std::endl;

		// If there is a password, return true
		if(count >= 0)
		{
			std::cout << "The table exists!" << std::endl;
			sqlite3_finalize(stmt);
			return true;
		}
	}

	std::cout << "The table doesn't exist" << std::endl;

	// sqlite3_finalize() frees up our SQL statement that we set up in sqlite3_prepare_v2()
	sqlite3_finalize(stmt);

	return false;
}

void DatabaseManager::createTable()
{
	// Construct a query that will create a table in our database
	std::string query = "CREATE TABLE swarchTable(userName VARCHAR(50), passWord VARCHAR(50), score INT)";

	// Prepares the SQL Query for use
	error = sqlite3_prepare_v2(db, query.c_str(), query.length(), &stmt, &tail);

	// If the SQL file wasn't prepared correctly, exit the method
	if(error != SQLITE_OK)
	{
		std::cout << "Couldn't prepare sql" << std::endl;
		sqlite3_finalize(stmt);
		return;
	}

	// Run our SQL program
	if(sqlite3_step(stmt) == SQLITE_ERROR)
	{
		std::cout << "Table failed to create" << std::endl;
		sqlite3_finalize(stmt);
		return;
	}

	std::cout << "Table created" << std::endl;

	// sqlite3_finalize() frees up our SQL statement that we set up in sqlite3_prepare_v2()
	sqlite3_finalize(stmt);
}

// Sends updates to the webserver regarding updated player info
std::list<PlayerScore> DatabaseManager::updateWebServer()
{
	// Construct a score list to be sent to the NetworkManager for transmission
	std::list<PlayerScore> scoreList;

	// Construct a query that will return all usernames and scores from within our table
	std::string query = "SELECT * FROM swarchTable";

	// Prepare the query for use by the sqlite
	error = sqlite3_prepare_v2(db, query.c_str(), query.length(), &stmt, &tail);

	// If an error occurred, return false
	if(error != SQLITE_OK)
	{
		std::cout << "Couldn't prepare sql" << std::endl;
		return scoreList;
	}

	// sqlite3_step() runs our query on the database
	while(sqlite3_step(stmt) == SQLITE_ROW)
	{
		// Acquires the password from the database (which we recast from a const char* to a string)
		std::string user = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
		int userScore = sqlite3_column_int(stmt, 2);

		// Save the name and score
		PlayerScore info;
		info.name = user;
		info.score = userScore;
		scoreList.push_back(info);
	}

	// sqlite3_finalize() frees up our SQL statement that we set up in sqlite3_prepare_v2()
	sqlite3_finalize(stmt);

	// Send the file
	return scoreList;
}