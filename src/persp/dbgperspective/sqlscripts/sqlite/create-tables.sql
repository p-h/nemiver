create table schemainfo (version text not null) ;

insert into schemainfo (version) values ('1.5') ;

create table sessions (id integer primary key) ;

create table env_variables (id integer primary key,
                            sessionid integer,
                            name text,
                            value text) ;

create table attributes (id integer primary key,
                         sessionid integer,
                         name text,
                         value text) ;

create table breakpoints (id integer primary key,
                          sessionid integer,
                          filename text,
                          filefullname text,
                          linenumber integer,
                          enabled integer,
                          condition text,
                          ignorecount integer,
			  iscountpoint integer) ;

create table watchpoints (id integer primary key,
                          sessionid integer,
                          expression text,
                          iswrite integer,
                          isread integer);

create table openedfiles (id integer primary key,
                          sessionid integer,
                          filename text) ;

create table searchpaths (id integer primary key,
                          sessionid integer,
                          path text) ;

