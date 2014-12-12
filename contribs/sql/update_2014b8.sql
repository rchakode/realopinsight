-- This script allows to update RealOpInsight Ultimate database
-- Target Version: v2014b7
-- Copyright (c) 2014 RealOpInsight Labs
-- Author Rodrigue Chakode <Rodrigue.Chakode@gmail.com>
-- Creation Date: 19/11/2014

-- alter table qosdata
CREATE TABLE "qosdata_upgraded" (
  "id" integer primary key autoincrement,
  "version" integer not null,
  "timestamp" bigint not null,
  "status" integer not null,
  "normal" real not null,
  "minor" real not null,
  "major" real not null,
  "critical" real not null,
  "unknown" real not null,
  "view_name" text,
 constraint "fk_qosdata_view" foreign key ("view_name") references "view" ("name") on delete cascade
);

INSERT INTO qosdata_upgraded SELECT * FROM qosdata;
DROP TABLE qosdata;
ALTER TABLE qosdata_upgraded RENAME TO qosdata;


-- alter table user_view
CREATE TABLE "user_view_upgraded" (
  "user_name" text,
  "view_name" text,
  primary key ("user_name", "view_name"),
 constraint "fk_user_view_key1" foreign key ("user_name") references "user" ("name") on delete cascade,
 constraint "fk_user_view_key2" foreign key ("view_name") references "view" ("name") on delete cascade
);

INSERT INTO user_view_upgraded SELECT * FROM user_view;
DROP TABLE user_view;
ALTER TABLE user_view_upgraded RENAME TO user_view;


-- create new table
CREATE TABLE "notification" (
  "id" integer primary key autoincrement,
  "version" integer not null,
  "timestamp" bigint not null,
  "view_status" integer not null,
  "ack_status" integer not null,
  "last_change" bigint not null,
  "view_name" text,
  "ack_user_name" text,
 constraint "fk_notification_view" foreign key ("view_name") references "view" ("name") on delete cascade,
 constraint "fk_notification_ack_user" foreign key ("ack_user_name") references "user" ("name") on delete cascade
);
