-- Active: 1763221774373@@127.0.0.1@5432@saedb
CREATE SCHEMA delivraptor;
SET SCHEMA 'delivraptor';

DROP TABLE IF EXISTS delivraptor.utilisateur, delivraptor.colis, delivraptor.liaison_colis_user;
CREATE TABLE delivraptor.utilisateur(
    bordereau BIGINT NOT NULL UNIQUE PRIMARY KEY,
    etape INT
)

CREATE TABLE delivraptor.colis(
    id SERIAL PRIMARY KEY
)

CREATE TABLE delivraptor.liaison_colis_user(
    id_colis BIGINT NOT NULL,
    id_utilisateur BIGINT NOT NULL,

    CONSTRAINT fk_liasion_colis FOREIGN KEY (id_colis)
        REFERENCES delivraptor.colis(id) ON DELETE CASCADE,

    CONSTRAINT fk_liaison_utilisateur FOREIGN KEY (id_utilisateur)
        REFERENCES delivraptor.utilisateur(bordereau) ON DELETE CASCADE

)

INSERT INTO delivraptor.utilisateur (bordereau, etape) 
    VALUES (452, '1');