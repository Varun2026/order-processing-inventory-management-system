USE order_system;

ALTER TABLE users
    ADD COLUMN IF NOT EXISTS password_hash VARCHAR(64) NOT NULL DEFAULT '';

UPDATE users SET password_hash = '4e40e8ffe0ee32fa53e139147ed559229a5930f89c2204706fc174beb36210b3'
    WHERE username = 'alice';

UPDATE users SET password_hash = '8d059c3640b97180dd2ee453e20d34ab0cb0f2eccbe87d01915a8e578a202b11'
    WHERE username = 'bob';

UPDATE users SET password_hash = '6868b751d7c664a9492079bfb2858c86cc95f9480270413e334ed09fe94cf10d'
    WHERE username = 'carol';

SELECT id, username, role, password_hash FROM users;
