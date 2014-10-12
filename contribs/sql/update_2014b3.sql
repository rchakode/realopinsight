-- This script allows to update RealOpInsight Ultimate database
-- from the version b1 or higher, to the version 2014b3
-- Copyright (c) 2014 RealOpInsight Labs
-- Author Rodrigue Chakode <Rodrigue.Chakode@gmail.com>
-- Creation Date: 12/10/2014

ALTER TABLE user ADD COLUMN authsystem int not null default 0;
