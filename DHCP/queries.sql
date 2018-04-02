
select c.ipv4, c.dns as d,
  c.enlace as e, c.mascara as m,
  d.enlace, d.mascara, d.dns from
  configuracion as c, datos_generales as d
  where c.mac = 246320239287372;


DELIMITER //
create procedure spObtenerConfiguracion(in address bigint)
begin

  select c.dns as d, c.enlace as e, c.mascara as m, c.ipv4,
    d.mascara, d.enlace, d.dns from configuracion as c,
    datos_generales as d where c.mac = address;

end //
DELIMITER ;


DELIMITER //
create procedure spObtenerIpv4(in address bigint)
begin

  declare existe int;

  set existe = (select ifnull(max(ipv4), 0) from ip_usadas where mac = address);
  if(existe > 0) then
    select ipv4 from ip_usadas where mac = address;
  else
    select ifnull(max(ipv4), 3232235876) + 1 as ipv4 from ip_usadas;
  end if ;

end //
DELIMITER ;


DELIMITER //
create procedure spRegistrarIP(in address bigint, in ip int)
begin

  declare existe int;

  set existe = (select count(mac) from ip_usadas where mac = address);
  if(existe = 1) then
    update ip_usadas set ipv4 = ip where mac = address;
  else
    insert into ip_usadas values(address, ip);
  end if ;

end //
DELIMITER ;



DELIMITER //
create procedure spEliminarIP(in address bigint)
begin

  delete from ip_usadas where mac = address;

end //
DELIMITER ;
