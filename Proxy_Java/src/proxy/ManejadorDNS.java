package proxy;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;

public class ManejadorDNS implements Tipos{
    
    private final String HOST = "8.8.8.8";
    private final int PUERTO = 53;
    private final int MAX_BUFFER = 1024;
    private final byte IP_HTTP[] = {8, 12, 0, 6};
    
    
    public byte[] crearRespuestaA(byte msg[], int tam) throws IOException{
        
        // incrementamos el answer count
        msg[6] = 0x00;
        msg[7] = 0x01;
        
        // ahora si, creamos los buffer para la respuesta
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        DataOutputStream dos = new DataOutputStream(baos);
        
        dos.write(msg, 0, tam);
        dos.flush();
        
        // ponemos el apuntador al nombre
        dos.writeShort(0xc00c);
        dos.flush();
        
        // ponemos el tipo de record, en este caso A
        dos.writeShort(0x0001);
        dos.flush();
        
        // ponemos la clase, in en este caso
        dos.writeShort(0x0001);
        dos.flush();
        
        // ponemos el ttl
        dos.writeInt(3000);
        dos.flush();
        
        // ponemos la longitud
        dos.writeShort(0x0004);
        dos.flush();
        
        // ponemos la ip del servidor http
        dos.write(IP_HTTP,0,4);
        dos.flush();
        
        return baos.toByteArray();
        
    }
    
    public void analizarMensaje(byte trama[], int tam) throws IOException{
        
        DataInputStream dis = new DataInputStream(new ByteArrayInputStream(trama));
        short auxiliar = 0;
        
        System.out.println("###############################");
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
            int aux = 0;
            long aux2 = 0;
            
            // esto esta feo, pero es para omitir el nombre
            dis.readShort();
            
            System.out.println("Respuesta:");
            
            aux = dis.readShort() & 0xFFFF;
            System.out.println("Tipo: " + aux);
            
            aux = dis.readShort() & 0xFFFF;
            System.out.println("Clase: " + aux);
            
            aux2 = dis.readInt() & 0xFFFFFFFF;
            System.out.println("Tipo: " + aux2);
        }
        
        System.out.println("###############################");
        
    }
    
    public void imprimirTrama(byte trama[], int tam){
        
        System.out.println("");
        for(int i = 0; i < tam;){
            String a = String.format("%x", trama[i]);
            String b = (a.length() == 1)?"0"+a:a;
            System.out.print( b + " ");
            if(++i % 16 == 0){
                System.out.println("");
            }
        }
        System.out.println("");
        
    }
    
    public DatagramPacket mandarPreguntaDNS(byte msg[], int tam) throws SocketException, IOException{
        
        System.out.println("Mandando: " + tam + " bytes.");
        
        DatagramSocket ds = new DatagramSocket();
        DatagramPacket dp = new DatagramPacket(msg, tam, InetAddress.getByName(HOST), PUERTO);
        ds.send(dp);
        
        DatagramPacket res = new DatagramPacket(new byte[MAX_BUFFER], MAX_BUFFER);
        ds.receive(res);
        
        System.out.println("Recibido: " + res.getLength());
       
        return res;
        
    }
    
    public Info obtenerDominioSolicitado(byte msg[]) throws IOException{
        
        DataInputStream dis = new DataInputStream(new ByteArrayInputStream(msg));
        dis.skipBytes(12);
        
        Info info = new Info();
        int plen = 0;
        String dominio = "";
        
        while((plen = (dis.readByte() & 0xFF)) > 0){

            byte nom[] = new byte[plen];
            for(int j = 0; j < plen; j++){
                nom[j] = dis.readByte();
            }
            dominio += new String(nom, 0, plen) + ".";
        }
        
        info.setNombre(dominio);
        info.setTipo(dis.readShort() & 0x00FF);
        
        return info;
        
    }
    
}
