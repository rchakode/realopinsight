-- This script allows to update RealOpInsight Ultimate database
-- Target Version: v2014b7
-- Copyright (c) 2014 RealOpInsight Labs
-- Author Rodrigue Chakode <Rodrigue.Chakode@gmail.com>
-- Creation Date: 19/11/2014


-- Modify table
-- table qosdata
ALTER TABLE qosdata DROP FOREIGN KEY fk_qosdata_view;
ALTER TABLE qosdata ADD constraint fk_qosdata_view
foreign key ("view_name") references "view" ("name") on delete cascade;

-- table user_view
ALTER TABLE user_view DROP FOREIGN KEY fk_user_view_key1;
ALTER TABLE user_view
   ADD constraint fk_user_view_key1
   foreign key ("user_name") references "user" ("name") on delete cascade;

ALTER TABLE user_view
   DROP FOREIGN KEY "fk_user_view_key2";
ALTER TABLE user_view
   ADD constraint "fk_user_view_key2"
   foreign key ("view_name") references "view" ("name") on delete cascade;


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

