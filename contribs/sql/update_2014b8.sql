-- This script allows to update RealOpInsight Ultimate database
-- Target Version: v2014b7
-- Copyright (c) 2014 RealOpInsight Labs
-- Author Rodrigue Chakode <Rodrigue.Chakode@gmail.com>
-- Creation Date: 19/11/2014

CREATE TABLE "notification" (
  "id" integer primary key autoincrement,
  "version" integer not null,
  "timestamp" bigint not null,
  "ack_status" integer not null,
  "ack_timestamp" bigint not null,
  "view_name" text,
  "ack_user_name" text,
 constraint "fk_notification_view" foreign key ("view_name") references "view" ("name"),
 constraint "fk_notification_ack_user" foreign key ("ack_user_name") references "user" ("name")
);
