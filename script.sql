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

CREATE TABLE client(
    identifiant VARCHAR(25) PRIMARY KEY,
    mot_de_passe VARCHAR(25)
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

INSERT INTO delivraptor.client (identifiant, mot_de_passe)
    VALUES 
    ('jean', '123!!'),
    ('Robert', '754?'),
    ('GupZoop', '45Ejkfsl'),
    ('Alizon', 'Super4');

SELECT * FROM delivraptor.utilisateur;
SELECT * FROM delivraptor.client 
    WHERE identifiant = 'jean' AND mot_de_passe = '123!!'