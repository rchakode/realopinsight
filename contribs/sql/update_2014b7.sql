-- This script allows to update RealOpInsight Ultimate database
-- from the version 2014b3 or higher, to the version 2014b7
-- Copyright (c) 2014 RealOpInsight Labs
-- Author Rodrigue Chakode <Rodrigue.Chakode@gmail.com>
-- Creation Date: 12/10/2014

CREATE TABLE "qosdata" (
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
 constraint "fk_qosdata_view" foreign key ("view_name") references "view" ("name")
);
