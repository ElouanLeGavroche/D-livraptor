-- Active: 1763221774373@@127.0.0.1@5432@saedb
CREATE SCHEMA delivraptor;
SET SCHEMA 'delivraptor';

DROP TABLE IF EXISTS delivraptor.utilisateur, delivraptor.logs, delivraptor.client;
CREATE TABLE delivraptor.utilisateur(
    bordereau BIGINT UNIQUE NOT NULL,
    etape INT
)

CREATE TABLE delivraptor.logs(
    id SERIAL PRIMARY KEY,
    time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

    id_utilisateur BIGINT UNIQUE NOT NULL,

    CONSTRAINT fk_utilisateur_logs FOREIGN KEY (id_utilisateur)
        REFERENCES delivraptor.utilisateur(bordereau)
);
CREATE TABLE client(
    identifiant VARCHAR(25) PRIMARY KEY,
    mot_de_passe VARCHAR(25)
);

CREATE OR REPLACE FUNCTION delivraptor.premier_log_func()
RETURNS TRIGGER AS $$
BEGIN
    INSERT INTO delivraptor.logs (id_utilisateur)
    VALUES (NEW.bordereau);

    RETURN NEW;
END;
$$ LANGUAGE plpgsql;


CREATE OR REPLACE TRIGGER premier_log AFTER INSERT ON delivraptor.utilisateur 
    FOR EACH ROW EXECUTE FUNCTION delivraptor.premier_log_func();

INSERT INTO delivraptor.client (identifiant, mot_de_passe)
    VALUES 
    ('jean', '123!!'),
    ('Robert', '754?'),
    ('GupZoop', '45Ejkfsl'),
    ('Alizon', 'Super4');


DELETE FROM delivraptor.logs;
DELETE FROM delivraptor.utilisateur;