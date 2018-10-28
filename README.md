RealOpinsight is an open-source operations monitoring system over Kubernetes®, Nagios®, Zabbix®, ManageEngine OpManager®, Pandora FMS®, Zenoss®, Icinga®, op5®, Centreon®, Shinken®, GroundWork®, and various other monitoring systems. 

The RealOpinsight benefit is to provide powerful aggregation and visualization for a comprehensive insight on your business applications.

# Installation 

See http://docs.realopinsight.com/

# Create PostgreSQL database for RealOpinsight

See official documentation for more details: https://www.postgresql.org/docs/9.3/static/tutorial-createdb.html

The following page would be also helpful: https://stackoverflow.com/questions/7695962/postgresql-password-authentication-failed-for-user-postgres

* Install the following packages:

  ```
  $ sudo apt-get install postgresql-client-9.5 postgresql-9.5
  ```

* Start PostgreSQL server

  ```
  $ sudo /etc/init.d/postgresql start
  ```

* Launch PostgreSQL prompt

  ```
  $ sudo -u postgres psql
  
  This will start a prompt as follows:
  postgres=#
  ```
  
* If it's a new installation of PostgreSQL, set a password for postgres user

  ```
  postgres=# ALTER USER postgres PASSWORD 'newPassword';
  ```

* Create a user for the RealOpInsight database and set a password

  ```
  postgres=# CREATE USER realopinsight;
  postgres=# ALTER USER realopinsight WITH ENCRYPTED PASSWORD '<password>';
  ```

  
* Create a database

  ```
  postgres=# CREATE DATABASE realopinsight;
  ```

  
* Granting privileges on database to the user

  ```
  postgres=# GRANT ALL PRIVILEGES ON DATABASE realopinsight TO realopinsight ;
  ```

# Security notes about integration with PostgreSQL

Note that the role in PostgreSQL **does not need**:

* to be a superuser
* to be allowed to create databases
* to be allowed to create more new roles



