-- WeCompany Server Database Setup Script
-- MySQL 5.7+ / MariaDB 10.2+

-- Create database
CREATE DATABASE IF NOT EXISTS wecompany 
  CHARACTER SET utf8mb4 
  COLLATE utf8mb4_unicode_ci;

USE wecompany;

-- Users table - stores account credentials
CREATE TABLE IF NOT EXISTS users (
    user_id VARCHAR(36) PRIMARY KEY COMMENT 'UUID user identifier',
    username VARCHAR(50) UNIQUE NOT NULL COMMENT 'Unique username for login',
    password_hash VARCHAR(64) NOT NULL COMMENT 'SHA-256 hashed password',
    salt VARCHAR(32) NOT NULL COMMENT 'Password salt for hashing',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT 'Account creation time',
    INDEX idx_username (username)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='User account credentials';

-- User profiles table - stores user information
CREATE TABLE IF NOT EXISTS user_profiles (
    user_id VARCHAR(36) PRIMARY KEY COMMENT 'User ID reference',
    nickname VARCHAR(100) COMMENT 'Display name/nickname',
    avatar VARCHAR(255) COMMENT 'Avatar image URL or path',
    email VARCHAR(100) COMMENT 'Email address',
    phone VARCHAR(20) COMMENT 'Phone number',
    status VARCHAR(20) DEFAULT 'offline' COMMENT 'User status: online, offline, busy, away',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT 'Profile creation time',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT 'Last update time',
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='User profile information';

-- Friend relations table - stores bidirectional friendships
CREATE TABLE IF NOT EXISTS friend_relations (
    relation_id VARCHAR(36) PRIMARY KEY COMMENT 'Unique relation ID',
    user_id VARCHAR(36) NOT NULL COMMENT 'User ID',
    friend_id VARCHAR(36) NOT NULL COMMENT 'Friend user ID',
    remark VARCHAR(100) COMMENT 'Friend remark/alias set by user',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT 'Friendship establishment time',
    UNIQUE KEY unique_friendship (user_id, friend_id),
    INDEX idx_user_id (user_id),
    INDEX idx_friend_id (friend_id),
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    FOREIGN KEY (friend_id) REFERENCES users(user_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Friend relationships';

-- Offline messages table - stores messages for offline users
CREATE TABLE IF NOT EXISTS offline_messages (
    message_id VARCHAR(36) PRIMARY KEY COMMENT 'Unique message ID',
    from_user_id VARCHAR(36) NOT NULL COMMENT 'Sender user ID',
    to_user_id VARCHAR(36) NOT NULL COMMENT 'Recipient user ID',
    content TEXT NOT NULL COMMENT 'Message content',
    message_type VARCHAR(20) DEFAULT 'text' COMMENT 'Message type: text, image, file, etc.',
    sent_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT 'Message sent time',
    delivered BOOLEAN DEFAULT FALSE COMMENT 'Delivery status',
    INDEX idx_to_user (to_user_id, delivered),
    INDEX idx_sent_at (sent_at),
    FOREIGN KEY (from_user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    FOREIGN KEY (to_user_id) REFERENCES users(user_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Offline message queue';

-- Create database user (optional)
-- Uncomment and modify the password before running
-- CREATE USER 'wecompany_user'@'localhost' IDENTIFIED BY 'your_secure_password';
-- GRANT ALL PRIVILEGES ON wecompany.* TO 'wecompany_user'@'localhost';
-- FLUSH PRIVILEGES;

-- Insert sample data (optional for testing)
-- Uncomment to create test users
/*
INSERT INTO users (user_id, username, password_hash, salt) VALUES
('test-user-001', 'alice', 'hash_here', 'salt_here'),
('test-user-002', 'bob', 'hash_here', 'salt_here');

INSERT INTO user_profiles (user_id, nickname, status) VALUES
('test-user-001', 'Alice', 'offline'),
('test-user-002', 'Bob', 'offline');

INSERT INTO friend_relations (relation_id, user_id, friend_id) VALUES
('relation-001', 'test-user-001', 'test-user-002'),
('relation-002', 'test-user-002', 'test-user-001');
*/

-- Show created tables
SHOW TABLES;

-- Display table structures
DESCRIBE users;
DESCRIBE user_profiles;
DESCRIBE friend_relations;
DESCRIBE offline_messages;

SELECT 'Database setup completed successfully!' AS Status;
