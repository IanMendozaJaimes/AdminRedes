package clientedns;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.ArrayList;


public class ManejadorDNS implements Tipos{
    
    private String HOST;
    private int PUERTO;
    private int MAX_BUFFER;
    private byte IP_HTTP[] = {127, 0, 0, 1};
    
    public ManejadorDNS(){
        //HOST = "148.204.103.2"; // servidor dns del ipn
        
        HOST = "8.8.8.8";
        PUERTO = 53;
        MAX_BUFFER = 1024;
        
    }
    
    public byte[] responder(byte query[], int tam) throws IOException{
        
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        DataOutputStream dos = new DataOutputStream(baos);
        
        for(int i = 0; i < query.length;){
            String a = String.format("%x", query[i]);
            String b = (a.length() == 1)?"0"+a:a;
            System.out.print( b + " ");
            if(++i % 16 == 0){
                System.out.println("");
            }

        }
        
        System.out.println("\n==========================");
        
        query[6] = 0x00;
        query[7] = 0x01;
        
        // iniciamos la respuesta con la peticion inicial
        dos.write(query, 0, tam);
        
        // escribimos el apuntador al nombre
        dos.writeShort(0xc00c);
        
        // decimos que el tipo de dato es A
        dos.writeShort(0x0001);
        
        // decimos que la clase es in
        dos.writeShort(0x0001);
        
        // decimos que el ttl
        dos.writeInt(0x1110);
        
        // decimos que la longitud es de 4 bytes
        dos.writeShort(0x0004);
        
        // ponemos la ip del servidor http
        dos.write(IP_HTTP);
        
        return baos.toByteArray();
        
    }
    
    public ArrayList<InfoRR> obtenerRR(String direccion, int tipo) throws IOException{
        
        ArrayList<InfoRR> lista = new ArrayList<>();
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        DataOutputStream dos = new DataOutputStream(baos);
        
        // escribiendo la cabecera generica de un mensaje DNS
        dos.write(generarCabeceraGenericaDNS());
        
        // escribiendo la pregunta
        dos.write(generarPregunta(direccion, tipo));
        byte cabecera[] = baos.toByteArray();
        
        // mandamos el mensaje
        byte respuesta[] = mandarPreguntaDNS(cabecera, cabecera.length);
        
        //imprimimos la respuesta
        imprimirRespuesta(respuesta, lista);
        
        return lista;
        
    }
    
    public String[] obtenerDominoSolicitado(byte query[]) throws IOException{
        
        String dominio[] = new String[2];
        
        dominio[0] = "";
        dominio[1] = "";
        
        DataInputStream dis = new DataInputStream(new ByteArrayInputStream(query));
        dis.skipBytes(12);
        
        int plen = 0;
        while((plen = (dis.readByte() & 0xFF)) > 0){

            byte nom[] = new byte[plen];
            for(int j = 0; j < plen; j++){
                nom[j] = dis.readByte();
            }
            dominio[0] += new String(nom, 0, plen) + ".";
        }
        
        int tipo = dis.readShort();
        
        dominio[1] = String.valueOf(tipo);
        
        return dominio;
        
    }
    
    public void imprimirRespuesta(byte res[], ArrayList<InfoRR> lista) throws IOException{
        
        DataInputStream dis = new DataInputStream(new ByteArrayInputStream(res));
        short auxiliar = 0;
            
            System.out.println("ID: " + dis.readShort());
            
            auxiliar = dis.readShort();
            System.out.println("QR: " + ((auxiliar >> 15) & 0x01));
            System.out.println("Opcode: " + ((auxiliar >> 11) & 0x0F));
            System.out.println("AA: " + ((auxiliar >> 10) & 0x01));
            System.out.println("TC: " + ((auxiliar >> 9) & 0x01));
            System.out.println("RD: " + ((auxiliar >> 8) & 0x01));
            System.out.println("RA: " + ((auxiliar >> 7) & 0x01));
            System.out.println("Z: " + ((auxiliar >> 4) & 0x07));
            System.out.println("RCode: " + (auxiliar & 0x0F));
            
            System.out.println("QD Count: " + dis.readShort());
            
            short anCount = dis.readShort();
            short arCount = dis.readShort();
            short adCount = dis.readShort();
            int totalRespuestas = anCount + arCount + adCount; 
            System.out.println("AN Count: " + anCount);
            System.out.println("Authority Record Count: " + arCount);
            System.out.println("Additional Record Count: " + adCount);
            
            //info del query
            int plen = 0;
            String n = "";
            while((plen = (dis.readByte() & 0xFF)) > 0){

                byte nom[] = new byte[plen];
                for(int j = 0; j < plen; j++){
                    nom[j] = dis.readByte();
                }
                n += new String(nom, 0, plen) + ".";
            }
            
            System.out.println("Name: " + n);
            System.out.println("Type: " + dis.readShort());
            System.out.println("Class: " + dis.readShort());
            
            for(int i = 0; i < totalRespuestas; i++){
                System.out.println("======================");
                int aux = 0;
                long aux2 = 0;
                InfoRR info = new InfoRR();
                info.setNombre(n.substring(0, n.length()-1));
                
                // esto esta feo, pero es para omitir el nombre
                dis.readShort();
                
                aux = dis.readShort() & 0xFFFF;
                info.setTipo(aux);
                
                aux = dis.readShort() & 0xFFFF;
                info.setClase(aux);
                
                aux2 = dis.readInt() & 0xFFFFFFFF;
                info.setTtl(aux2);
                
                
                System.out.println("Type: " + info.getTipo());
                System.out.println("Class: " + info.getClase());
                System.out.println("TTL: " + info.getTtl());
                
                if(info.getTipo() == A){
                    analizarARecord(info, dis);
                }
                else if(info.getTipo() == AAAA){
                    analizarAAAARecord(info, dis);
                }
                else{
                    System.out.println("No soporto el tipo: " + info.getTipo() + "... aun");
                }
                
                System.out.println("Data: " + info.getDatos());
                
                lista.add(info);
            }

    }
    
    private void analizarAAAARecord(InfoRR info, DataInputStream dis) throws IOException{
        int len = dis.readShort() & 0xFF;
        int aux = 0;
        String ip = "";
        System.out.println("DLength: " + len);
        
        for(int i = 0; i < len; i++){
            aux = dis.readByte() & 0xFF;
            ip += String.format("%x", aux);
            ip += (i % 2 != 0)?":":"";
        }
        
        info.setDatos(ip.substring(0,ip.length()-1));
    }
    
    private void analizarARecord(InfoRR info, DataInputStream dis) throws IOException{
        int len = dis.readShort() & 0xFF;
        int aux = 0;
        String ip = "";
        System.out.println("DLength: " + len);
        
        for(int i = 0; i < len; i++){
            aux = dis.readByte() & 0xFF;
            ip += aux + ".";
        }
        
        info.setDatos(ip.substring(0,ip.length()-1));
    }
    
    public byte[] mandarPreguntaDNS(byte msg[], int tam) throws SocketException, UnknownHostException, IOException{
        
        System.out.println("Mandando: " + msg.length + " bytes.");
        DatagramSocket ds = new DatagramSocket();
        DatagramPacket dp = new DatagramPacket(msg, tam, InetAddress.getByName(HOST), PUERTO);
        ds.send(dp);
        
        DatagramPacket res = new DatagramPacket(new byte[MAX_BUFFER], MAX_BUFFER);
        ds.receive(res);
        
        System.out.println("Recibido: " + res.getLength());
       
        return res.getData();
        
    }
    
    public byte[] generarCabeceraGenericaDNS() throws IOException{
        
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        DataOutputStream dos = new DataOutputStream(baos);
        
        // poniendo el id en 1
        dos.writeShort(0x00001);
        
        // poniendo las banderas, queremos recursion asi que...
        dos.writeShort(0x0100);
        
        // numero de preguntas
        dos.writeShort(0x0001);
        
        // numero de respuestas
        dos.writeShort(0x0000);
        
        // numero de nombres de servicio en la seccion de autoridad
        dos.writeShort(0x0000);
        
        //records adicionales
        dos.writeShort(0x0000);
        
        return baos.toByteArray();
        
    }
    
    public byte[] generarPregunta(String direccion, int tipo) throws IOException{
        
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        DataOutputStream dos = new DataOutputStream(baos);
        
        String partesDireccion[] = direccion.split("\\.");
        System.out.println("La url: " + direccion + ", tiene " + partesDireccion.length + " partes.");
        
        for(String parte:partesDireccion){
            byte b[] = parte.getBytes();
            dos.writeByte(b.length);
            dos.write(b);
        }
        
        // ya no hay mas que poner
        dos.writeByte(0x00);
        
        // tipo de mensaje
        dos.writeShort(tipo);
        
        // clase
        dos.writeShort(0x0001);
        
        return baos.toByteArray();
        
    }
    
}
