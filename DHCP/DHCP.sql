-- MySQL Workbench Forward Engineering

SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='TRADITIONAL,ALLOW_INVALID_DATES';

-- -----------------------------------------------------
-- Schema DHCP
-- -----------------------------------------------------

-- -----------------------------------------------------
-- Schema DHCP
-- -----------------------------------------------------
CREATE SCHEMA IF NOT EXISTS `DHCP` DEFAULT CHARACTER SET utf8 ;
USE `DHCP` ;

-- -----------------------------------------------------
-- Table `DHCP`.`DHCP_CI`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `DHCP`.`DHCP_CI` (
  `macAdress` VARCHAR(7) NOT NULL,
  `ipv4` TINYINT(1) NULL,
  `mascara` TINYINT(1) NULL,
  `puertaEnlace` TINYINT(1) NULL,
  `DNS` TINYINT(1) NULL,
  PRIMARY KEY (`macAdress`))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `DHCP`.`IPv4`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `DHCP`.`IPv4` (
  `macAdress` VARCHAR(7) NOT NULL,
  `IPv4` VARCHAR(4) NULL,
  PRIMARY KEY (`macAdress`))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `DHCP`.`Configuraciones`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `DHCP`.`Configuraciones` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `mascara` VARCHAR(4) NULL,
  `puertaEnlace` VARCHAR(4) NULL,
  `DNS` VARCHAR(4) NULL,
  PRIMARY KEY (`id`))
ENGINE = InnoDB;


SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
