-- This script allows to update RealOpInsight Ultimate database
-- Target Version: v2014b8
-- Copyright (c) 2015 RealOpInsight Labs
-- Author Rodrigue Chakode <Rodrigue.Chakode@gmail.com>
-- Creation Date: 18/02/2015

-- alter table user

ALTER TABLE user ADD COLUMN dashboardDisplayMode int not null default 0;
ALTER TABLE user ADD COLUMN dashboardTilesPerRow int not null default 5;
