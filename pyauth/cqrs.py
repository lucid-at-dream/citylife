import sqlite3

class Query:

    def __init__(self, commands):
        """
        :param commands: The list of commands that the query requires
        """
        self.commands = commands

    def fetch(self, ):

class Command:
    """
    An events
    """

    def __init__(self, name, args):
        """
        :param name:   Event name
        :param domain: Event domain name
        :param args:   Json representing the event arguments
        """
        self.name = name
        self.args = args

    def __hash__(self):
        return (str(self.name) + str(self.domain) + str(self.args)).__hash__()


class CommandStore:

    def __init__(self, db):
        self.db = db
        self.db.execute('''CREATE TABLE IF NOT EXISTS events (
                event_id varchar PRIMARY KEY AUTOINCREMENT NOT NULL,
                event_data timestamp,
                name varchar INDEX,
                args text
        )''')


class AuthStorage:

    def __init__(self, db):
        self.db = db

    def create_db_if_not_exists(self):
        self.db.execute('''CREATE TABLE IF NOT EXISTS users (username varchar unique PRIMARY KEY, password varchar)''')
        self.db.execute('''CREATE TABLE IF NOT EXISTS groups (name varchar unique PRIMARY KEY, description text)''')
        self.db.execute('''CREATE TABLE IF NOT EXISTS user_groups (user varchar, group varchar, 
                FOREIGN KEY (user) REFERENCES users(username) ON DELETE CASCADE,
                FOREIGN KEY (group) REFERENCES users(name) ON DELETE CASCADE)''')

    def add_user(self, user, password):
        self.db.execute('INSERT INTO users (username, password) VALUES ({0}, {1})'.format(user, password))

    def delete_user(self, user):
        self.db.execute('DELETE FROM users WHERE username = "{0}")'.format(user))

    def change_password(self, user, password):
        self.db.execute('UPDATE users WHERE username = "{0}" SET password = "{1}")'.format(user, password))

    def add_group(self, group, description="A group of users"):
        self.db.execute('INSERT INTO group (name, description) VALUES ({0}, {1})'.format(group, description))

    def remove_group(self, group):
        self.db.execute('DELETE FROM group WHERE name="{0}"'.format(group))

    def add_user_to_group(self, user, group):
        self.db.execute('INSERT INTO user_groups (user, group) VALUES ({0}, {1})'.format(user, group))

    def delete_user_from_group(self, user, group):
        self.db.execute('DELETE FROM user_groups WHERE user="{0}" and group="{1}'.format(user, group))



class SessionStorage:

    def create_db_if_not_exists(self):
        pass

    def start_session(self, user, max_duration):
        pass

    def validate_session(self, session):
        pass

    def end_session(self, session):
        pass


if __name__ == '__main__':
    auth_conn = sqlite3.connect('auth.db')
    auth_db = auth_conn.cursor()

    session_conn = sqlite3.connect('session.db')
    session_db = session_conn.cursor()
